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

// The page count threshold of using CONFIG_LARGE_PAGE_SIZE instead of CONFIG_PAGE_SIZE
#define PAGE_COUNT_THRESHOLD (CONFIG_LARGE_PAGE_SIZE/CONFIG_PAGE_SIZE)*4

extern "C" void jump_to_kernel_main(LoaderArgument *loader_argument , max_t new_stack_addr);

extern "C" __no_sanitize_address__ __kernel_setup_text__ 
void kernel_setup(LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        while(1) { ; }
    }

    // If kernel's not configured to be higher-half, ignore all the setup stage and immediately jump to kernel main
#if CONFIG_KERNEL_HIGHERHALF == no
    kernel_main(loader_argument);
#endif

#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::init(loader_argument);
    debug::out::clear_screen(0x00);
    debug::out::printf("Setting up kernel's memory space...\n");
#endif
    LoaderMemoryMap *lmemmap = (LoaderMemoryMap*)loader_argument->memmap_location;
    page::init_pt_space_allocator(loader_argument);

    // Calculate the maximum memory address 
    max_t maximum_memory_addr = 0;
    for(int i = 0; i < loader_argument->memmap_count; i++) {
        max_t addr = ((max_t)lmemmap[i].addr_high << (sizeof(lmemmap[i].addr_high)*8))|lmemmap[i].addr_low;
        max_t len  = ((max_t)lmemmap[i].length_high << (sizeof(lmemmap[i].length_high)*8))|lmemmap[i].length_low;
        
        maximum_memory_addr = max(maximum_memory_addr , addr+len);
    }

    // determine page size, use large page if applicable

    max_t kernel_size = loader_argument->kernel_size;
    max_t kernel_page_count = align_round_up(kernel_size , DEFAULT_PAGE_SIZE)/DEFAULT_PAGE_SIZE;

    max_t kernel_stack_page_count = align_round_up(loader_argument->kernel_stack_size , CONFIG_PAGE_SIZE)/CONFIG_PAGE_SIZE;

#ifdef ENABLE_DEBUG_FUNCTIONS
    debug::out::printf("Kernel size       : %dkB\n" , kernel_size/1024);
    debug::out::printf("Page-aligned kernel size : %d pages\n" , kernel_page_count);
    debug::out::printf("Kernel stack size : %dkB\n" , CONFIG_KERNEL_STACK_SIZE/1024);
    debug::out::printf("Kernel stack page count  : %d pages\n" , kernel_stack_page_count);
#endif

    PageTableData page_table_data;

    // map the kernel onto the higher-half address
    max_t kernel_linear_address = CONFIG_KERNEL_VMADDRESS+loader_argument->kernel_physical_location;
    page::map_pages(
        page_table_data , 
        kernel_linear_address , 
        DEFAULT_PAGE_SIZE , kernel_page_count , 
        loader_argument->kernel_physical_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    // map the kernel stack onto the higher-half address
    max_t kernel_stack_linear_address = CONFIG_KERNEL_VMADDRESS+loader_argument->kernel_physical_location+(kernel_page_count*DEFAULT_PAGE_SIZE);
    page::map_pages(
        page_table_data , 
        kernel_stack_linear_address , 
        CONFIG_PAGE_SIZE , kernel_stack_page_count , 
        loader_argument->kernel_stack_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    // set identity paging
    page::map_pages(
        page_table_data , 
        0x00 , 
        CONFIG_LARGE_PAGE_SIZE , (maximum_memory_addr/(CONFIG_LARGE_PAGE_SIZE))+1 , 
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
            DEFAULT_PAGE_SIZE , 
            align_round_up(loader_argument->dbg_graphic_framebuffer_end-loader_argument->dbg_graphic_framebuffer_start , DEFAULT_PAGE_SIZE) , 
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
            DEFAULT_PAGE_SIZE , 
            align_round_up(loader_argument->dbg_graphic_framebuffer_end-loader_argument->dbg_graphic_framebuffer_start , DEFAULT_PAGE_SIZE) , 
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
        (kernel_pool_end-kernel_pool_start)/1024/1024/1024 , 
        ((kernel_pool_end-kernel_pool_start)/1024/1024/100)%10 , 
        ((kernel_pool_end-kernel_pool_start)/1024/1024/10)%10);
#endif
    
    // Now that everything is setted up
    jump_to_kernel_main(loader_argument , (kernel_stack_linear_address+kernel_stack_page_count*CONFIG_PAGE_SIZE)-8);
    while(1) {
        ;
    }
}
