#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/sections.hpp>

#include <loader/loader_argument.hpp>
#include <loader/kernel_setup_argument.hpp>
#include <kernel/mem/pages_manager.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>

// The page count threshold of using CONFIG_LARGE_PAGE_SIZE instead of CONFIG_PAGE_SIZE
#define PAGE_COUNT_THRESHOLD (CONFIG_LARGE_PAGE_SIZE/CONFIG_PAGE_SIZE)*4

extern "C" void jump_to_kernel_main(LoaderArgument *loader_argument , max_t new_stack_address);
max_t check_alignment(max_t address , int va_count , ...);

extern "C" __no_sanitize_address__ __kernel_setup_text__ 
void kernel_setup(LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        while(1) { ; }
    }

    // If kernel's not configured to be higher-half, ignore all the setup stage and immediately jump to kernel main
#if CONFIG_KERNEL_HIGHERHALF == no
    kernel_main(loader_argument);
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
    PageTableData page_table_data;

    // map the kernel onto the higher-half address
    max_t kernel_linear_address = CONFIG_KERNEL_VMADDRESS+loader_argument->kernel_physical_location;
    max_t kernel_page_size = check_alignment(loader_argument->kernel_physical_location , 2 , CONFIG_PAGE_SIZE , CONFIG_LARGE_PAGE_SIZE);
    max_t kernel_page_count = align_round_up(kernel_size , kernel_page_size)/kernel_page_size;
    page::map_pages(
        page_table_data , 
        kernel_linear_address , 
        kernel_page_size , kernel_page_count , 
        loader_argument->kernel_physical_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );
    // map the kernel stack onto the higher-half address
    max_t kernel_stack_linear_address = CONFIG_KERNEL_VMADDRESS+loader_argument->kernel_physical_location+(kernel_page_count*DEFAULT_PAGE_SIZE);
    max_t kernel_stack_page_size = check_alignment(kernel_stack_linear_address , 2 , CONFIG_PAGE_SIZE , CONFIG_LARGE_PAGE_SIZE);
    max_t kernel_stack_page_count = align_round_up(loader_argument->kernel_stack_size , kernel_stack_page_size)/kernel_stack_page_size;
    page::map_pages(
        page_table_data , 
        kernel_stack_linear_address , 
        kernel_stack_page_size , kernel_stack_page_count , 
        loader_argument->kernel_stack_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    // set identity paging
    page::map_pages(
        page_table_data , 
        loader_argument->kernel_physical_location , 
        CONFIG_LARGE_PAGE_SIZE , (maximum_memory_addr/(CONFIG_LARGE_PAGE_SIZE))+1 , 
        loader_argument->kernel_physical_location , 
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
    // Add the kernel setup argument at the end of the kernel stack

    max_t kernel_stack_address = (kernel_stack_linear_address+kernel_stack_page_count*CONFIG_PAGE_SIZE)-WORD_SIZE;
    // Setup kernel setup argument
    auto [start , end] = page::get_pt_space_boundary();
    
    // Set up the loader_argument
    loader_argument->pt_space_start = start;
    loader_argument->pt_space_end   = end;
    
    jump_to_kernel_main(loader_argument , kernel_stack_address);
    while(1) {
        ;
    }
}

/// @brief Given the list of the values, return the value that divides the address
/// @param address 
/// @param va_count Number of elements in the list 
/// @param va       List of the alignments (type : max_t)
/// @return The first value from the VA list that divides the address with remainder of 0
///         If unable to find, the function returns 0.
__kernel_setup_text__
max_t check_alignment(max_t address , int va_count , ...) {
    va_list ap;
    va_start(ap , address);
    for(int i = 0; i < va_count; i++) {
        max_t val = va_arg(ap , max_t);
        if(address%val == 0) return val;
    }
    va_end(ap);
    return 0;
}