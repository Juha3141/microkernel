#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/kasan.hpp>
#include <kernel/mem/pages_manager.hpp>

#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/io_port.hpp>

#include <kernel/debug.hpp>

#include <kernel/sections.hpp>
#include <loader/loader_argument.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

__no_sanitize_address__ max_t setup_kasan_shadowmem(LoaderArgument *loader_argument , PageTableData &page_table_data);
__no_sanitize_address__ max_t get_kernel_memory_pool_size();

// + Add kernel_setup argument that tells the virtual addresses of important memory areas like stack, loader_argument and kstruct etc.
extern "C" void kernel_main(LoaderArgument *loader_argument) {
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    debug::init(loader_argument);
    
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the higher-half kernel!\n");
    
    debug::out::printf("Kernel Setup Page Table Space : 0x%-10llx ~ 0x%-10llx\n" , loader_argument->pt_space_start , loader_argument->pt_space_end);

    memory::kmemmap_init(loader_argument);
    memory::add_kmemmap_entry((KernelMemoryMap){
        .start_address = loader_argument->pt_space_start , 
        .end_address   = loader_argument->pt_space_end , 
        .type          = MEMORYMAP_KERNEL_PT_SPACE
    });
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();
    /* To-do : 
     * add KASan initialization here before the pmem_init
     */

    // Very very temporary!!
    PageTableData page_table_data = {
        .cr3_base = (x86_page_entry_t *)loader_argument->pt_space_start
    };
    setup_kasan_shadowmem(loader_argument , page_table_data);

    debug::out::printf("========================== Kernel memory map ==========================\n");
    while(kmemmap_ptr != nullptr) {
        debug::out::printf("0x%-16llx ~ 0x%-16llx % 13lldkB (%s)\n" , kmemmap_ptr->start_address , kmemmap_ptr->end_address , (kmemmap_ptr->end_address-kmemmap_ptr->start_address)/1024 , memory::memmap_type_to_str(kmemmap_ptr->type));
        kmemmap_ptr = kmemmap_ptr->next;
    }

    memory::pmem_init();

    debug::out::printf(DEBUG_INFO , "----- Initializing segmentation system..\n");
    segmentation::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing interrupt system..\n");
    interrupt::init();
    exception::init();

    debug::out::printf("We're currently in safe mode\n");

    while(1) {
        ;
    }
}

#define PAGE_COUNT_THRESHOLD (CONFIG_LARGE_PAGE_SIZE/CONFIG_PAGE_SIZE)*4

/// @brief get_kernel_memory_pool_size() scans the size of the available memory that kernel will use as a free pool from the
///        kernel's global memory map(global_kmemmap)
///        The size of the memory pool is in multiple of CONFIG_PAGE_SIZE. That is, each chunks of the memory is rounded down to the
///        nearest multiple of the CONFIG_PAGE_SIZE. (align_round_down)
/// @return Size of the available kernel memory (NOT number of pages)
__no_sanitize_address__ 
max_t get_kernel_memory_pool_size() {
    max_t kernel_memory_pool_size = 0;
    KernelMemoryMap *ptr = memory::global_kmemmap();
    while(ptr != nullptr) {
        max_t len = (ptr->end_address-ptr->start_address);
        // skip if the size of the memory region is smaller than page size
        if(ptr->type != MEMORYMAP_USABLE) { ptr = ptr->next; continue; }

        kernel_memory_pool_size += align_round_down(len , CONFIG_PAGE_SIZE);
        ptr = ptr->next;
    }
    return kernel_memory_pool_size;
}

__no_sanitize_address__
max_t setup_kasan_shadowmem(LoaderArgument *loader_argument , PageTableData &page_table_data) {
    // Get a space for KASan first
    max_t kernel_memory_pool_size = get_kernel_memory_pool_size();
    max_t linear_address_mapping_location = CONFIG_KERNEL_KASAN_VMA;
    max_t kasan_shadowmem_size = align_round_up(kernel_memory_pool_size/(KASAN_GRANUL_SIZE+1) , DEFAULT_PAGE_SIZE);
    debug::out::printf("Kernel memory pool size  : %dMB\n" , kernel_memory_pool_size/1024/1024);
    debug::out::printf("KASan shadow memory size : %dMB\n" , kasan_shadowmem_size/1024/1024);
    
    // variable tracking how many number of pages the system has mapped
    max_t mapped_memory_size = 0;
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();
    while(kmemmap_ptr != nullptr) {
        max_t addr_start = kmemmap_ptr->start_address;
        max_t addr_end   = kmemmap_ptr->end_address;
        // skip if the size of the memory region is smaller than page size
        if(kmemmap_ptr->type != MEMORYMAP_USABLE) { kmemmap_ptr = kmemmap_ptr->next; continue; }
        debug::out::printf("Address change : 0x%-10llx ~ 0x%-10llx" , addr_start , addr_end);
        /* Determine the page size
         * If the memory chunk(kmemmap_ptr) has more than PAGE_COUNT_THRESHOLD number of page with the size of CONFIG_PAGE_SIZE,
         * it will use the CONFIG_LARGE_PAGE_SIZE as the default page size.
        */
        max_t page_size = CONFIG_PAGE_SIZE;
        // number of pages >= PAGE_COUNT_THRESHOLD
        if((addr_end-addr_start)/page_size >= PAGE_COUNT_THRESHOLD)  page_size = CONFIG_LARGE_PAGE_SIZE;
        addr_start = align_round_up(addr_start , page_size);
        addr_end = align_round_down(addr_end , page_size);

        debug::out::printf(" --> 0x%-10llx ~ 0x%-10llx\n" , addr_start , addr_end);

        // the number of pagess that will be mapped for the current memory chunk
        max_t chunk_map_size = min(mapped_memory_size+addr_end-addr_start , kasan_shadowmem_size)-mapped_memory_size;
        debug::out::printf("chunk map size  : %lldkB\n" , chunk_map_size/1024);
        debug::out::printf("how much mapped : %lldkB\n" , mapped_memory_size/1024);
        mapped_memory_size += chunk_map_size;

        /** TO-DO :
         * Check alignment of the linear address mapping location before determining the page size
         * Make an intelligent algorithm that determines how to determine page size efficiently 
         * Also consider the PT space, maybe you might want to deprecate it and change it to something that's more fitting to main kernel space
         */
        page::map_pages(
            page_table_data , 
            linear_address_mapping_location , 
            page_size , chunk_map_size/page_size , 
            addr_start , 
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
            page::alloc_pt_space
        );
        
        if(!memory::add_kmemmap_entry((KernelMemoryMap){addr_start , addr_start+chunk_map_size , MEMORYMAP_KASAN_SHADOWMEM})) {
            debug::panic_line(__FILE_NAME__ , __LINE__ , "add_kmemmap_entry() failed, arg1: %llx, arg2: %llx, arg3: %d\n" , 
                addr_start , addr_start+chunk_map_size , MEMORYMAP_KASAN_SHADOWMEM);
        }

        debug::out::printf("KASan shadow memory mapping : 0x%llx ~ 0x%llx  -->  0x%llx ~ 0x%llx (%d pages, ps=%d)\n" , 
            addr_start , addr_start+chunk_map_size , 
            linear_address_mapping_location , linear_address_mapping_location+chunk_map_size , chunk_map_size/page_size , page_size);

        linear_address_mapping_location += chunk_map_size;

        // If the mapped page count is bigger than the number of pages of the calculated size of shadowmem, 
        // we have completed mapping all the shadowmem.
        if(mapped_memory_size >= kasan_shadowmem_size) {
            break;
        }
    }

    debug::out::printf("Total mapped size           : %dkB (%d.%d%d%%)\n" , mapped_memory_size/1024 , 
        ((mapped_memory_size*100)/kernel_memory_pool_size) , ((mapped_memory_size*1000)/kernel_memory_pool_size)%10 , ((mapped_memory_size*10000)/kernel_memory_pool_size)%10);

    return kasan_shadowmem_size;
}