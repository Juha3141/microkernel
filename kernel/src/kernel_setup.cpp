#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/sections.hpp>

#include <loader/loader_argument.hpp>
#include <kernel/mem/pages_manager.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>

// The page count threshold of using CONFIG_LARGE_PAGE_SIZE instead of CONFIG_PAGE_SIZE
#define PAGE_COUNT_THRESHOLD (CONFIG_LARGE_PAGE_SIZE/CONFIG_PAGE_SIZE)*4

extern "C" void jump_to_kernel_main(LoaderArgument *loader_argument , max_t kernel_vma , max_t kernel_stack_vma , max_t kernel_stack_size , max_t pt_space_addr , max_t pt_space_size);
max_t check_alignment(max_t address , int va_count , ...);

__kernel_setup_text__ 
extern "C" void kernel_setup(LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        while(1) { ; }
    }

    // If kernel's not configured to be higher-half, ignore all the setup stage and immediately jump to kernel main
#if CONFIG_KERNEL_HIGHERHALF == no
    kernel_main(loader_argument , 0 , 0 , 0);
#endif
    LoaderMemoryMap *lmemmap = (LoaderMemoryMap*)loader_argument->memmap_location;
    page::init_pt_space_allocator(loader_argument);

    // Calculate the maximum memory address 
    max_t maximum_memory_addr = 0;
    max_t maximum_kernel_structure_addr = 0;
    for(int i = 0; i < loader_argument->memmap_count; i++) {
        max_t addr = ((max_t)lmemmap[i].addr_high << (sizeof(lmemmap[i].addr_high)*8))|lmemmap[i].addr_low;
        max_t len  = ((max_t)lmemmap[i].length_high << (sizeof(lmemmap[i].length_high)*8))|lmemmap[i].length_low;
        
        maximum_memory_addr = max(maximum_memory_addr , addr+len);
    }
    // Kernel image
    maximum_kernel_structure_addr = max(maximum_kernel_structure_addr
        , loader_argument->kernel_physical_location+loader_argument->kernel_size);
    // Kernel stack
    maximum_kernel_structure_addr = max(maximum_kernel_structure_addr
        , loader_argument->kernel_stack_location+CONFIG_KERNEL_STACK_SIZE);
    // Loader argument
    maximum_kernel_structure_addr = max(maximum_kernel_structure_addr
        , loader_argument->loader_argument_location+loader_argument->loader_argument_size);
    // The memory map itself
    maximum_kernel_structure_addr = max(maximum_kernel_structure_addr
        , loader_argument->memmap_location+loader_argument->memmap_count*sizeof(LoaderMemoryMap));

    // Map the entire physical address directly to the CONFIG_KERNEL_VMA
    max_t page_size = 
#if CONFIG_USE_ENORMOUS_PAGE == yes
    CONFIG_ENORMOUS_PAGE_SIZE;
#elif CONFIG_USE_LARGE_PAGE == yes
    CONFIG_LARGE_PAGE_SIZE;
#else 
    CONFIG_PAGE_SIZE;
#endif
    // map the kernel onto the higher-half address
    max_t kernel_area_ps = check_alignment(maximum_kernel_structure_addr , 2 , CONFIG_PAGE_SIZE , CONFIG_LARGE_PAGE_SIZE);
    page::map_pages(
        page::kernel_page_table() , 
        0x00 ,  
        kernel_area_ps , 
        align_round_up(maximum_kernel_structure_addr,  kernel_area_ps)/kernel_area_ps , 
        0x00 , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );
    
    // map ramdisk onto the higher-half address
    max_t ramdisk_area_ps = check_alignment(loader_argument->ramdisk_location , 2 , CONFIG_PAGE_SIZE , CONFIG_LARGE_PAGE_SIZE);
    page::map_pages(
        page::kernel_page_table() , 
        CONFIG_KERNEL_RAMDISK_VMA , 
        ramdisk_area_ps , 
        align_round_up(loader_argument->ramdisk_size , ramdisk_area_ps)/ramdisk_area_ps , 
        loader_argument->ramdisk_location , 
        PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );
    
    // identity-map the page table space, use default page size
    auto [pt_space_start, pt_space_end] = page::get_pt_space_boundary();

    // directly map the whole RAM to CONFIG_KERNEL_VMADDRESS
    page::map_pages(
        page::kernel_page_table() , 
        CONFIG_KERNEL_VMADDRESS , 
        page_size , 
        align_round_up(maximum_memory_addr,  page_size)/page_size ,
        0x00 , 
        PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_RW , 
        page::alloc_pt_space
    );

    page::register_page_table(page::kernel_page_table());
    // Tell everybody that we're now in the higher half kernel!
    page::higherhalf_is_now_configured();

    // Add the kernel setup argument at the end of the kernel stack
    jump_to_kernel_main(loader_argument , 
        TO_VMEM(loader_argument->kernel_physical_location) , 
        TO_VMEM(loader_argument->kernel_stack_location) , 
        loader_argument->kernel_stack_size , 
        pt_space_start , 
        pt_space_end);
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