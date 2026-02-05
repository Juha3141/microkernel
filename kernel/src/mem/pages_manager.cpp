#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kmem_manager.hpp>

__attribute__ ((section(".kernel_setup_stage"))) struct {
    max_t start_addr;
    max_t end_addr;
    max_t current_addr;

    max_t growth_size;
}kernel_pt_space_manager;

__no_sanitize_address__ constexpr bool is_inside_boundary(max_t addr , const memory::Boundary &boundary) {
    return (boundary.start_address <= addr && addr < boundary.end_address);
}

/// @brief Initialize the allocator that governs the space for kernel's page table
///        The kernel will choose the biggest memory chunk from the global kernel memory map 
///        and set the start of the page table allocator's heap to the chunk's start address
///        The size of this heap will be the size of one large page. If large page is not available, 
///        the system uses the default CONFIG_PAGE_SIZE
__no_sanitize_address__ void page::init_pt_space_allocator() {
    KernelMemoryMap *ptr = memory::global_kmemmap();
    KernelMemoryMap *entry_with_max_size = nullptr;
    while(ptr != nullptr) {
        // Choose the largest memory region that's available
        if(ptr->type == MEMORYMAP_USABLE && 
        (entry_with_max_size->end_address-entry_with_max_size->start_address) < (ptr->end_address-ptr->start_address)) {
            entry_with_max_size = ptr;
        }

        ptr = ptr->next;
    }
    // no available memory, highly unlikely
    if(entry_with_max_size == nullptr) return;

    // the size of the page table space is the size of one page. Use large page size if applicable
    kernel_pt_space_manager.start_addr      = entry_with_max_size->start_address;
    kernel_pt_space_manager.growth_size     =
#ifdef CONFIG_USE_LARGE_PAGE 
        +CONFIG_LARGE_PAGE_SIZE;
#else
        +CONFIG_PAGE_SIZE;
#endif
    kernel_pt_space_manager.end_addr        = entry_with_max_size->start_address
                                            + kernel_pt_space_manager.growth_size;
    kernel_pt_space_manager.current_addr    = kernel_pt_space_manager.start_addr;
    // debug::out::printf("Kernel's page table space : 0x%llx ~ 0x%llx\n" , pt_space_start , pt_space_end);
    memory::add_kmemmap_entry((KernelMemoryMap){
        kernel_pt_space_manager.start_addr , 
        kernel_pt_space_manager.end_addr , 
        MEMORYMAP_KERNEL_PT_SPACE
    });
}

/// @brief Rudimentary allocator for kernel's page table
__no_sanitize_address__ void *page::alloc_pt_space(max_t size , max_t alignment) {
	max_t addr = align_round_up(kernel_pt_space_manager.current_addr , alignment); // Align address
	kernel_pt_space_manager.current_addr = addr+size; // increment address
    if(kernel_pt_space_manager.current_addr >= kernel_pt_space_manager.end_addr) {
        // Enlarge the memory area for page table by growth_size in the kernel_pt_space_manager.
        memory::add_kmemmap_entry((KernelMemoryMap){
            kernel_pt_space_manager.end_addr , 
            kernel_pt_space_manager.end_addr+kernel_pt_space_manager.growth_size , 
            MEMORYMAP_KERNEL_PT_SPACE
        });
        kernel_pt_space_manager.end_addr += kernel_pt_space_manager.growth_size;
    }

	return (void *)addr;
}

memory::Boundary page::get_pt_space_boundary(void) {
    return {kernel_pt_space_manager.start_addr , kernel_pt_space_manager.current_addr};
}


__no_sanitize_address__ void page::map_pages(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func) {
    for(max_t i = 0; i < page_count; i++) {
        map_one_page(page_table_data , linear_addr+(i*page_size) , page_size , physical_address+(i*page_size) , flags , alloc_func);
    }
}