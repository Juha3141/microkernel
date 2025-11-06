#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kmem_manager.hpp>

__attribute__ ((section(".kernel_setup_stage"))) struct {
    max_t start_addr;
    max_t end_addr;
    max_t current_addr;
}kernel_pt_space_manager;

constexpr bool is_inside_boundary(max_t addr , const memory::Boundary &boundary) {
    return (boundary.start_address <= addr && addr < boundary.end_address);
}

/// @brief Initialize the allocator that governs the space for kernel's page table
///        The kernel will choose the biggest memory chunk from the provided loader argument's memory map 
///        and set the start of the allocator's heap to the chunk's start address
/// @param loader_argument The loader argument from bootloader
void page::init_pt_space_allocator(LoaderArgument *loader_argument) {
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
	protected_areas[3].end_address   = loader_argument->memmap_location+memory::align_address((loader_argument->memmap_count*sizeof(struct MemoryMap)) , 4096);
	// 5. kstruct Memory Area
	protected_areas[4].start_address = loader_argument->kstruct_mem_location;
	protected_areas[4].end_address   = loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size;
    
    MemoryMap *memmap = (MemoryMap *)((max_t)loader_argument->memmap_location);
    
    long max_size_chunk_id = -1;
    max_t pt_space_start;
    max_t pt_space_end;

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
}

/// @brief Rudimentary allocator for kernel's page table
max_t page::alloc_pt_space(max_t size , max_t alignment) {
	max_t addr = memory::align_address(kernel_pt_space_manager.current_addr , alignment); // Align address
	kernel_pt_space_manager.current_addr = addr+size; // increment address
	if(kernel_pt_space_manager.current_addr >= kernel_pt_space_manager.end_addr) return 0x00;
    
	return addr;
}

void page::init_higher_half(MemoryMap *memmap , max_t memmap_count , max_t higherhalf_relocation_address) {
    
}