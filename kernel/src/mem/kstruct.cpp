#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <kernel/debug.hpp>
#include <string.hpp>

// Just temporary patch
struct {
	Boundary boundary;
	max_t current_addr;
}kstruct_mgr;

void memory::get_kstruct_boundary(Boundary &boundary) {
	memcpy(&boundary , &kstruct_mgr.boundary , sizeof(Boundary));
}

Boundary get_largest_memory_chunk(LoaderArgument *loader_argument);

__no_sanitize_address__ void memory::kstruct_init(LoaderArgument *loader_argument) {
    /* kstruct allocates objects from the rear of the largest memory chunk in the loader argument */
    /* Thus it shares the code with pt_space initializer. Specifically, kstruct_init also uses get_largest_memory_chunk() */
    auto [chunk_phys_start , chunk_phys_end] = get_largest_memory_chunk(loader_argument);

	kstruct_mgr.boundary.start_address = TO_VMEM(chunk_phys_start);
	kstruct_mgr.boundary.end_address   = TO_VMEM(chunk_phys_end);
    kstruct_mgr.current_addr = kstruct_mgr.boundary.end_address;
}

__no_sanitize_address__ void *memory::kstruct_alloc(max_t size , max_t alignment) {
    max_t alloc_addr = kstruct_mgr.current_addr-size;

	alloc_addr = align_round_down(alloc_addr , alignment); // Align address
	kstruct_mgr.current_addr = alloc_addr;

	if(kstruct_mgr.current_addr <= kstruct_mgr.boundary.start_address) {
		debug::panic("kmem_manager.cpp" , 40 , "kstruct_alloc() : full kernel struct space\n");
	}
	return (void *)alloc_addr;
}

__no_sanitize_address__ bool memory::is_kstruct_allocated_obj(void *obj) {
	return kstruct_mgr.current_addr <= (max_t)obj && (max_t)obj <= kstruct_mgr.boundary.end_address;
}

max_t memory::kstruct_get_current_addr(void) { return kstruct_mgr.current_addr; }