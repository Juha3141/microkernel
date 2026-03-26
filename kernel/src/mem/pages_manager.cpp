#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kmem_manager.hpp>

__kernel_setup_data__ struct {
    max_t start_addr;
    max_t end_addr;
    max_t current_addr;

}kernel_pt_space_manager;

__kernel_setup_text__ 
static inline bool is_inside_boundary(max_t addr , const memory::Boundary &boundary) {
    return (boundary.start_address <= addr && addr <= boundary.end_address);
}

/// @brief Initialize the allocator that governs the space for kernel's page table
///        The kernel will choose the biggest memory chunk from the global kernel memory map 
///        and set the start of the page table allocator's heap to the chunk's start address
///        The size of this heap will be the size of one large page. If large page is not available, 
///        the system uses the default CONFIG_PAGE_SIZE
__kernel_setup_text__ 
bool page::init_pt_space_allocator(LoaderArgument *loader_argument) {
    KernelMemoryMap essential_memmap_boundaries[] = essential_kernel_mem_boundaries(loader_argument);
    LoaderMemoryMap *lmemmap = (LoaderMemoryMap *)((max_t)loader_argument->memmap_location);
    max_t chunk_start = 0;
    max_t chunk_end = 0;

    for(unsigned int i = 0; i < loader_argument->memmap_count; i++) {
        max_t addr = ((max_t)lmemmap[i].addr_high << (sizeof(lmemmap[i].addr_high)*8))|lmemmap[i].addr_low;
        max_t len  = ((max_t)lmemmap[i].length_high << (sizeof(lmemmap[i].length_high)*8))|lmemmap[i].length_low;
        if(lmemmap[i].type != MEMORYMAP_USABLE||len == 0) continue;

        bool chunk_not_available = false;
        for(int j = 0; j < sizeof(essential_memmap_boundaries)/sizeof(KernelMemoryMap); j++) {
            KernelMemoryMap &chunk = essential_memmap_boundaries[j];
            if(is_inside_boundary(chunk.start_address , {addr , addr+len})||is_inside_boundary(chunk.end_address , {addr , addr+len})) {
                chunk_not_available = true;
                break;
            }
        }

        if(len > chunk_end-chunk_start && !chunk_not_available) {
            chunk_start = addr;
            chunk_end   = addr+len;
        }
    }
    if(chunk_start == 0 && chunk_end == 0) {
        return false;
    }

    // Tentative size
    kernel_pt_space_manager.start_addr = chunk_start;
    kernel_pt_space_manager.end_addr   = chunk_end;
    kernel_pt_space_manager.current_addr = chunk_start;

    return true;
}

/// @brief Rudimentary allocator for kernel's page table
__kernel_setup_text__
void *page::alloc_pt_space(max_t size , max_t alignment) {
	max_t addr = align_round_up(kernel_pt_space_manager.current_addr , alignment); // Align address
	kernel_pt_space_manager.current_addr = addr+size; // increment address
    if(kernel_pt_space_manager.current_addr >= kernel_pt_space_manager.end_addr) return nullptr;

	return (void *)addr;
}
__kernel_setup_text__
memory::Boundary page::get_pt_space_boundary(void) {
    return {kernel_pt_space_manager.start_addr , kernel_pt_space_manager.current_addr};
}


// To-do : create something similar to kmemmap_manager that manages what memory is mapped to which
// Maybe it would be a good idea to store it in the page_table_data? or centralized system?
__kernel_setup_text__ bool page::map_pages(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func) {
    for(max_t i = 0; i < page_count; i++) {
        if(!map_one_page(page_table_data , linear_addr+(i*page_size) , page_size , physical_address+(i*page_size) , flags , alloc_func)) return false;
    }
    return true;
}