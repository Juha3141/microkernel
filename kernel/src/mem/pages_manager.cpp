#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kmem_manager.hpp>

__attribute__ ((section(".kernel_setup_stage"))) struct {
    max_t start_addr;
    max_t end_addr;
    max_t current_addr;
}kernel_pt_space_manager;

__no_sanitize_address__ constexpr bool is_inside_boundary(max_t addr , const memory::Boundary &boundary) {
    return (boundary.start_address <= addr && addr < boundary.end_address);
}

/// @brief Initialize the allocator that governs the space for kernel's page table
///        The kernel will choose the biggest memory chunk from the provided loader_argument's memory map 
///        and set the start of the page table allocator's heap to the chunk's start address
/// @param loader_argument The loader argument from bootloader
__no_sanitize_address__ void page::init_pt_space_allocator(LoaderArgument *loader_argument) {
	memory::Boundary protected_areas[5];
	// 1. Kernel
	protected_areas[0].start_address = loader_argument->kernel_physical_location;
	protected_areas[0].end_address   = loader_argument->kernel_physical_location+loader_argument->kernel_size;
	// 2. Stack
	protected_areas[1].start_address = loader_argument->kernel_stack_location;
	protected_areas[1].end_address   = loader_argument->kernel_stack_location+loader_argument->kernel_stack_size;
	// 3. Loader Argument Location	
	protected_areas[2].start_address = loader_argument->loader_argument_location;
	protected_areas[2].end_address   = loader_argument->loader_argument_location+loader_argument->loader_argument_size;
	// 4. Kernel Memory Map
	protected_areas[3].start_address = loader_argument->memmap_location;
	protected_areas[3].end_address   = loader_argument->memmap_location+align_round_up((loader_argument->memmap_count*sizeof(struct LoaderMemoryMap)) , 4096);
	// 5. kstruct Memory Area
	protected_areas[4].start_address = loader_argument->kstruct_mem_location;
	protected_areas[4].end_address   = loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size;
    
    LoaderMemoryMap *memmap = (LoaderMemoryMap *)((max_t)loader_argument->memmap_location);
    
    long max_size_chunk_id = -1;
    max_t pt_space_start = 0;
    max_t pt_space_end = 0;

    // Find the memory with the largest contiguous length
    for(long i = (long)loader_argument->memmap_count-1; i >= 0; i--) {
        if(memmap[i].type != MEMORYMAP_USABLE) continue;
        max_t addr = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
        max_t len  = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low; 
        if((pt_space_end-pt_space_start) < len) {
            pt_space_start = addr;
            pt_space_end   = addr+len;
            max_size_chunk_id = i;
        }
    }
    // Check if address is safe,
    // move the start address of the page table space to the end of the protected memory, if it's inside of the protective memory.
    // Continue moving the start address until the memory area is safe to use
    bool is_addr_safe;
    do {
        is_addr_safe = true;
        for(const memory::Boundary &b : protected_areas) {
            if(is_inside_boundary(pt_space_start , b)) {
                is_addr_safe = false;
                pt_space_start = b.end_address;
            }
        }
    }while(!is_addr_safe);

    kernel_pt_space_manager.start_addr      = pt_space_start;
    kernel_pt_space_manager.end_addr        = pt_space_end;
    kernel_pt_space_manager.current_addr    = pt_space_start;
    // debug::out::printf("Kernel's page table space : 0x%llx ~ 0x%llx\n" , pt_space_start , pt_space_end);
}

/// @brief Rudimentary allocator for kernel's page table
__no_sanitize_address__ void *page::alloc_pt_space(max_t size , max_t alignment) {
	max_t addr = align_round_up(kernel_pt_space_manager.current_addr , alignment); // Align address
	kernel_pt_space_manager.current_addr = addr+size; // increment address
	if(kernel_pt_space_manager.current_addr >= kernel_pt_space_manager.end_addr) {
        return 0x00;
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