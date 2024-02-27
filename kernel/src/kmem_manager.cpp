/**
 * @file kmem_manager.cpp
 * @brief Core system of kernel memory manager
 * 
 * @author Ian Juha Cho
 * contact : ianisnumber2027@gmail.com
 */

#include <kernel/kmem_manager.hpp>
#include <kernel/nodes_manager.hpp>

#include <string.hpp>

#include <kernel/debug.hpp>

// for C++ standard

void *operator new(max_t size) { return memory::pmem_alloc(size); }
void *operator new[](max_t size) { return memory::pmem_alloc(size); }
void operator delete(void *ptr , max_t) { memory::pmem_free(ptr); }

// Just temporary patch
struct {
	struct memory::Boundary boundary;
	max_t current_addr;
}kstruct_mgr;

void memory::get_kstruct_boundary(struct Boundary &boundary) {
	memcpy(&boundary , &kstruct_mgr.boundary , sizeof(struct Boundary));
}

void memory::determine_kstruct_boundary(struct memory::Boundary &new_mboundary , struct KernelArgument *kargument) {
	debug::push_function("d_kstruct_b");
	struct MemoryMap *memmap = (struct MemoryMap *)((max_t)kargument->memmap_ptr);
	max_t kernel_end_address = kargument->total_kernel_area_end;
	debug::out::printf("kernel_end_address : 0x%X\n" , kernel_end_address);
	new_mboundary.start_address = align_address(kernel_end_address , 4096); // padding
	/* The "kstruct" is location of all sorts of kernel structures that requires static memory location.
	 * The kernel makes a space about 1MB large at the rear of kernel image for static structures.
	 * This code (haha) basically searches what "memmap[i]" has the kernel image and calculates whether
	 * the kstruct area is available to be created.
	 */
	for(int i = 0; i < kargument->memmap_count; i++) {
		max_t address = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
		max_t length = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
		// debug::out::printf("seg%d : 0x%X~0x%X\n" , i , address , address+length);
		if(kernel_end_address >= address && kernel_end_address <= address+length
		&& kargument->total_kernel_area_start >= address && kargument->total_kernel_area_start <= address+length) { // is kernel inside?
			// determine boundary
			// If segment cannot hold more than designated size, just use maximum size for the segment
			new_mboundary.end_address = MIN(address+length , new_mboundary.start_address+KERNELSTRUCTURE_LENGTH);
			debug::out::printf(DEBUG_INFO , "kstruct_boundary : 0x%X~0x%X\n" , new_mboundary.start_address , new_mboundary.end_address);
			debug::pop_function();
			return;
		}
	}
	new_mboundary.end_address = new_mboundary.start_address+KERNELSTRUCTURE_LENGTH;
	debug::out::printf(DEBUG_INFO , "def,kstruct_boundary : 0x%X~0x%X\n" , new_mboundary.start_address , new_mboundary.end_address);
	debug::pop_function();
	return;
}

int memory::determine_pmem_boundary(struct Boundary protect , struct Boundary *new_msegment_list , struct KernelArgument *kargument) {
	debug::push_function("d_pmem_b");

	int seg_index = 0;
	struct MemoryMap *memmap = (struct MemoryMap *)((max_t)kargument->memmap_ptr);
	struct Boundary kstruct_boundary;
	get_kstruct_boundary(kstruct_boundary);
	// unnecessary but necessary I guess
	/*
	 * This is just parsing the protected area from the segment
	 * Basically, this code excludes "protect" boundary from segment that encompasses that boundary
	 * The segment splices the designated the segment(memmap[i]) into two parts which is usable(and not protected)
	 * ..and saves the address of the segments to new_msegment_list
	 * ..effectively excluding kernel boundary from physical memory pool
	*/
	for(int i = 0; i < kargument->memmap_count; i++) {
		max_t address = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
		max_t length = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
		if(memmap[i].type != MEMORYMAP_USABLE) continue;
		// If the segment is larger than certain "threshold" -> save it 
		if(protect.start_address >= address && protect.start_address <= address+length && protect.start_address-address >= KERNEL_MEMORY_SEGMENT_THRESHOLD) {
			max_t new_addr = address;
			max_t new_length = protect.start_address-address;
			
			new_msegment_list[seg_index].start_address = new_addr;
			new_msegment_list[seg_index].end_address = new_addr+new_length;
			debug::out::printf("seg%d. 0x%lX~0x%lX\n" , seg_index , new_addr , new_addr+new_length);
			seg_index++;

		}
		if(protect.end_address >= address && protect.end_address <= address+length) {
			max_t new_addr = align_address(protect.end_address , 4096); // padding
			max_t new_length = address+length-protect.end_address;

			new_msegment_list[seg_index].start_address = new_addr;
			new_msegment_list[seg_index].end_address = new_addr+new_length;
			debug::out::printf("seg%d. 0x%lX~0x%lX\n" , seg_index , new_addr , new_addr+new_length);
			seg_index++;
		}
		else if(protect.start_address <= address+length) {
			debug::out::printf("seg%d. 0x%lX~0x%lX\n" , seg_index , address , address+length);
			
			new_msegment_list[seg_index].start_address = address;
			new_msegment_list[seg_index].end_address = address+length;
			seg_index++;
		}
	}
	debug::pop_function();
	return seg_index;
}

void memory::kstruct_init(struct memory::Boundary boundary) {
	memcpy(&kstruct_mgr.boundary , &boundary , sizeof(struct Boundary));
	kstruct_mgr.current_addr = boundary.start_address;
}

void *memory::kstruct_alloc(max_t size , max_t alignment) {
	max_t addr = align_address(kstruct_mgr.current_addr , alignment); // Align address
	kstruct_mgr.current_addr = addr+size; // increment address
	if(kstruct_mgr.current_addr >= kstruct_mgr.boundary.end_address) {
		debug::panic("kmem_manager.cpp" , 40 , "kstruct_alloc() : full kernel struct space\n");
	}
	return (void *)addr;
}

void memory::SegmentsManager::init(int segment_count , struct Boundary *usable_segments) {
	managers_count = segment_count;
	total_memory = 0;
	// allocate space for all managers
	node_managers = (NodesManager *)kstruct_alloc(managers_count*sizeof(NodesManager));
	for(int i = 0; i < managers_count; i++) {
		node_managers[i].init(usable_segments[i].start_address , usable_segments[i].end_address-usable_segments[i].start_address);
		total_memory += (usable_segments[i].end_address-usable_segments[i].start_address);
	}
}

int memory::SegmentsManager::get_segment_index(max_t address) {
	for(int i = 0; i < managers_count; i++) {
		if(address >= node_managers[i].mem_start_address && address <= node_managers[i].mem_end_address) {
			return i;
		}
	}
	return -1;
}

void memory::pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct KernelArgument *kargument) {
	debug::push_function("pmem_init");
	
	int usable_seg_count = 0;

	struct Boundary kstruct_boundary;
	struct Boundary pmem_boundary[memmap_count];
	int pmem_entry_count = 0;

	// determine kernel struct address
	determine_kstruct_boundary(kstruct_boundary , kargument);
	// determine physical memory address
	if((pmem_entry_count = determine_pmem_boundary({kargument->total_kernel_area_start , kstruct_boundary.end_address} , pmem_boundary , kargument)) == 0) {
		debug::panic("kmem_manager.cpp" , 55 , "pmem_init() : zero detected usable memory\n");
	}
	kstruct_init(kstruct_boundary); // initialize kernel structure area
	// allocate node manager for each segment
	SegmentsManager *segments_mgr = SegmentsManager::get_self();
	segments_mgr->init(pmem_entry_count , pmem_boundary);
	debug::out::printf("Total memory : %dMB\n" , segments_mgr->total_memory/1024/1024);
	debug::pop_function();
}

max_t memory::SegmentsManager::get_currently_using_mem(void) {
	max_t currently_using_mem = 0;
	for(int i = 0; i < managers_count; i++) {
		currently_using_mem += node_managers[i].currently_using_mem;
		debug::out::printf("node_managers[%d].currently_using_mem : %d\n" , i , node_managers[i].currently_using_mem);
	}
	return currently_using_mem;
}

vptr_t *memory::pmem_alloc(max_t size , max_t alignment) {
	vptr_t *ptr = 0x00;
	debug::push_function("pmem_alloc");
	SegmentsManager *segments_mgr = SegmentsManager::get_self();
	for(int i = 0; i < segments_mgr->managers_count; i++) {
		if(!segments_mgr->node_managers[i].available()) continue;
		if((ptr = (vptr_t *)segments_mgr->node_managers[i].allocate(size , alignment)) != 0x00) {
			break;
		}
	}
	if(ptr == 0x00) {
		debug::panic("no available physical memory\n");
	}
	debug::pop_function();
	return ptr;
}

void memory::pmem_free(vptr_t *ptr) {
	debug::push_function("pmem_free");
	SegmentsManager *segments_mgr = SegmentsManager::get_self();
	int index;
	if((index = segments_mgr->get_segment_index((max_t)ptr)) == -1) {
		debug::out::printf("Warning : Memory release request out of range(0x%lX)\n" , (max_t)ptr);
		debug::pop_function();
		return;
	}
	if(segments_mgr->node_managers[index].free((max_t)ptr) == false) {
		debug::out::printf("Warning : Memory release request not allocated(0x%lX)\n" , (max_t)ptr);
	}
	debug::pop_function();
}

bool memory::pmem_protect(struct Boundary boundary) {
	return false;
}

max_t memory::pmem_usage(void) { return GLOBAL_OBJECT(SegmentsManager)->get_currently_using_mem(); }