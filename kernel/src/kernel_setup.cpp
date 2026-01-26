#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/sections.hpp>

#include <loader/loader_argument.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kasan.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

#define ENABLE_DEBUG_FUNCTIONS

extern "C" void kernel_main(LoaderArgument *loader_argument);
max_t get_kernel_memory_pool_size(KernelMemoryMap *memmap , max_t memmap_count , max_t page_size);
void  map_loader_argument_video_memory(LoaderArgument *loader_argument , KernelMemoryMap *memmap , max_t memmap_count , PageTableData &page_table_data , max_t page_size);

KernelMemoryMap *revise_memory_map(LoaderArgument *loader_argument);

extern "C" __no_sanitize_address__ __entry_function__ void kernel_setup(struct LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        while(1) { ; }
    }
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    
#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::init(loader_argument);
    debug::out::clear_screen(0x00);
    debug::out::printf("Setting up kernel's memory space...\n");
#endif
    memory::kmemmap_init(loader_argument);

    // determine page size, use large page if applicable
    max_t page_size = 
#if CONFIG_USE_LARGE_PAGE == yes
            CONFIG_LARGE_PAGE_SIZE;
#else
            CONFIG_PAGE_SIZE;
#endif

    KernelMemoryMap *ptr = memory::global_kmemmap();
    while(ptr != nullptr) {
        if(ptr->start_address == 0x00 && ptr->end_address == 0x00) { ptr = ptr->next; continue; }
        debug::out::printf("0x%llx ~ 0x%llx (%s)\n" , ptr->start_address , ptr->end_address , memory::memmap_type_to_str(ptr->type));

        ptr = ptr->next;
    }

    extern char __kernel_start__, __kernel_end__;
    max_t kernel_size = (&__kernel_end__-&__kernel_start__);
    max_t kernel_page_count = align_round_up(kernel_size , page_size)/page_size;

    max_t kernel_stack_page_count = align_round_up(loader_argument->kernel_stack_size , page_size)/page_size;

#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::out::printf("Kernel size       : %dkB\n" , kernel_size/1024);
    debug::out::printf("Page-aligned kernel size : %d pages\n" , kernel_page_count);
    debug::out::printf("Kernel stack size : %dkB\n" , CONFIG_KERNEL_STACK_SIZE/1024);
    debug::out::printf("Kernel stack page count  : %d pages\n" , kernel_stack_page_count);
#endif

    page::init_pt_space_allocator(loader_argument);
    PageTableData page_table_data;

    KernelMemoryMap *kernel_memmap = memory::global_kmemmap();

    // TEMPORARY!!!!
    max_t maximum_memory_addr = 0;
    max_t linear_address_mapping_location 
        = CONFIG_KERNEL_HIGHERHALF_ADDRESS+
        loader_argument->kernel_physical_location+(kernel_page_count*page_size)+
        CONFIG_KERNEL_STACK_SIZE;
    max_t kernel_pool_start = linear_address_mapping_location;

    max_t kernel_memory_pool_size = get_kernel_memory_pool_size(kernel_memmap , loader_argument->memmap_count , page_size);
#ifdef CONFIG_USE_KASAN
    // Get a space for KASan first
    max_t kasan_linear_addr_start = CONFIG_KERNEL_KASAN_VMA;
    max_t kasan_size = kernel_memory_pool_size/(KASAN_GRANUL_SIZE+1);
    max_t kasan_page_count = align_round_up(kasan_size , page_size)/page_size;
    kernel_memory_pool_size -= kasan_size;
#endif


#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::out::printf("KASan shadow memory size  : %lldMB\n" , kasan_size/1024/1024);
    debug::out::printf("KASan linear address      : 0x%llx\n" , kasan_linear_addr_start);
#endif

    max_t mapped_page_count = 0;
    size_t i = 0;
#ifdef CONFIG_USE_KASAN 
    debug::out::printf("memmap count : %d\n" , loader_argument->memmap_count);
    for(; i < loader_argument->memmap_count; i++) {
        max_t len = (kernel_memmap[i].end_address-kernel_memmap[i].start_address);
        max_t addr = kernel_memmap[i].start_address;
        max_t region_page_count = (len/page_size);
        maximum_memory_addr = MAX(maximum_memory_addr , len+addr);

        addr = align_round_up(addr , page_size);
        if(region_page_count == 0)             continue;
        if(kernel_memmap[i].type != MEMORYMAP_USABLE) continue;
        // if the memory segment is where the kernel is physically loaded, skip it
        if(loader_argument->kernel_physical_location >= addr && addr+len >= loader_argument->kernel_physical_location) continue;
        
        max_t actual_page_count = MIN(mapped_page_count+region_page_count , kasan_page_count)-mapped_page_count;
        mapped_page_count += actual_page_count;

        page::map_pages(
            page_table_data , 
            kasan_linear_addr_start , 
            page_size , actual_page_count , 
            addr , 
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
            page::alloc_pt_space
        );
        debug::out::printf("KASan shadow memory mapping : 0x%llx ~ 0x%llx  -->  0x%llx ~ 0x%llx (%d pages)\n" , 
            addr , addr+actual_page_count*(page_size) , 
            kasan_linear_addr_start , kasan_linear_addr_start+actual_page_count*(page_size) , actual_page_count);
        
        kasan_linear_addr_start += actual_page_count*page_size;

        // Mapped all the necessary KASan Address space
        if(mapped_page_count >= kasan_page_count) {
            page::map_pages(
                page_table_data , 
                linear_address_mapping_location , 
                page_size , region_page_count-actual_page_count , 
                addr + (actual_page_count*page_size) , 
                PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
                page::alloc_pt_space
            );
            i++;
            debug::out::printf("(KASAN residual) Memory 0x%llX ~ 0x%llX, page count : %d ===> Mapped to 0x%016llX\n" , 
                addr+(actual_page_count*page_size) , 
                addr+(region_page_count*page_size) , 
                region_page_count-actual_page_count , 
                linear_address_mapping_location);
            
            linear_address_mapping_location += (region_page_count-actual_page_count)*page_size;
            break;
        }
    }
#endif

    for(; i < loader_argument->memmap_count; i++) {
        max_t len = kernel_memmap[i].end_address - kernel_memmap[i].start_address;
        max_t addr = kernel_memmap[i].start_address;
        addr = align_round_up(addr , page_size);
        maximum_memory_addr = MAX(maximum_memory_addr , len+addr);

        max_t region_page_count = len/page_size;
        if(region_page_count == 0)             continue;  // skip if the size of the memory region is either zero or smaller than page size
        if(kernel_memmap[i].type != MEMORYMAP_USABLE) continue;
#ifdef ENABLE_DEBUG_FUNCTIONS
        debug::out::printf("Memory 0x%llX ~ 0x%llX, page count : %d ===> Mapped to 0x%016llX\n" , addr , addr+len , region_page_count , linear_address_mapping_location);
#endif

        page::map_pages(
            page_table_data , 
            linear_address_mapping_location , // linear address
            page_size , region_page_count , // page size and number of pages to be mapped onto the provided addresses
            addr ,                            // physical address
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
            page::alloc_pt_space
        );

        linear_address_mapping_location += region_page_count*page_size;
    }
    max_t kernel_pool_end = linear_address_mapping_location;
#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::out::printf("Maximum memory address : 0x%llx\n" , maximum_memory_addr);
    debug::out::printf("Kernel memory pool size : %dMB\n" , kernel_memory_pool_size/1024/1024);
#endif

    // map the kernel onto the higher-half address
    page::map_pages(
        page_table_data , 
        CONFIG_KERNEL_HIGHERHALF_ADDRESS+loader_argument->kernel_physical_location , 
        page_size , kernel_page_count , 
        loader_argument->kernel_physical_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    // map the kernel stack onto the higher-half address
    page::map_pages(
        page_table_data , 
        CONFIG_KERNEL_HIGHERHALF_ADDRESS+loader_argument->kernel_physical_location+kernel_page_count*page_size , 
        page_size , kernel_stack_page_count , 
        loader_argument->kernel_stack_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    // set identity paging
    page::map_pages(
        page_table_data , 
        0x00 , 
        page_size , (maximum_memory_addr/(page_size))+1 , 
        0x00 , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );
    // identity-map the video memory (If it's not covered by maximum_memory_addr)
    if(((loader_argument->video_mode & LOADER_ARGUMENT_VIDEOMODE_GRAPHIC) == LOADER_ARGUMENT_VIDEOMODE_GRAPHIC)
    && loader_argument->dbg_graphic_framebuffer_end >= maximum_memory_addr) {
        page::map_pages(
            page_table_data , 
            loader_argument->dbg_graphic_framebuffer_start , 
            page_size , 
            align_round_up(loader_argument->dbg_graphic_framebuffer_end-loader_argument->dbg_graphic_framebuffer_start , page_size) , 
            loader_argument->dbg_graphic_framebuffer_end , 
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
            page::alloc_pt_space
        );
    }
    if(((loader_argument->video_mode & LOADER_ARGUMENT_VIDEOMODE_TEXTMODE) == LOADER_ARGUMENT_VIDEOMODE_TEXTMODE)
    && loader_argument->dbg_graphic_framebuffer_end >= maximum_memory_addr) {
        page::map_pages(
            page_table_data , 
            loader_argument->dbg_graphic_framebuffer_start , 
            page_size , 
            align_round_up(loader_argument->dbg_graphic_framebuffer_end-loader_argument->dbg_graphic_framebuffer_start , page_size) , 
            loader_argument->dbg_graphic_framebuffer_end , 
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
            page::alloc_pt_space
        );
    }
    
    page::register_page_table(page_table_data);

    auto [start , end] = page::get_pt_space_boundary();
#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::out::printf("Memory used for page table : 0x%llx ~ 0x%llx (%dkB)\n" , start , end , (end-start)/1024);
    debug::out::printf("kernel_main location : 0x%X\n" , kernel_main);
    
    debug::out::printf("Linearly mapped free pool : 0x%llx ~ 0x%llx (%d.%d%dGBs)\n" , 
        kernel_pool_start , kernel_pool_end , 
        (kernel_pool_end-kernel_pool_start)/1000/1000/1000 , 
        ((kernel_pool_end-kernel_pool_start)/1000/1000/100)%10 , 
        ((kernel_pool_end-kernel_pool_start)/1000/1000/10)%10);
#endif

#if 0
    // debug::out::printf("Setting up KASan...\n");
    unsigned char *kvma = (unsigned char *)CONFIG_KERNEL_KASAN_VMA;
    for(max_t i = 0; i < kasan_size; i++) {
        kvma[i] = 69;
    }

    kasan::init(kasan_size , CONFIG_KERNEL_HIGHERHALF_ADDRESS , kernel_pool_end);
#endif

    while(1) {
        ;
    }
}

__no_sanitize_address__ max_t get_kernel_memory_pool_size(KernelMemoryMap *memmap , max_t memmap_count , max_t page_size) {
    max_t kernel_memory_pool_size = 0;
    for(size_t i = 0; i < memmap_count; i++) {
        max_t len = (memmap[i].end_address-memmap[i].start_address);
        max_t region_page_count = len/page_size;
        if(region_page_count == 0) { continue; } // skip if the size of the memory region is either zero or smaller than page size
        if(memmap[i].type != MEMORYMAP_USABLE) continue;
        // debug::out::printf("Memory 0x%llX ~ 0x%llX, page count : %d ===> Mapped to 0x%016llX\n" , addr , addr+len , region_page_count , linear_address_mapping_location);

        kernel_memory_pool_size += len;
    }
    return kernel_memory_pool_size;
}