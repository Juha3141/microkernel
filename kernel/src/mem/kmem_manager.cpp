/**
 * @file kmem_manager.cpp
 * @brief Core system of kernel memory manager
 * 
 * @author Ian Juha Cho
 * contact : ianisnumber2027@gmail.com
 */

#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/nodes_manager.hpp>
#include <loader/loader_argument.hpp>

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
bool is_pmem_alloc_available = false;

void memory::get_kstruct_boundary(struct Boundary &boundary) {
	memcpy(&boundary , &kstruct_mgr.boundary , sizeof(struct Boundary));
}
/*
void memory::determine_kstruct_boundary(struct memory::Boundary &new_mboundary , struct LoaderArgument *loader_argument) {
	debug::push_function("d_kstruct_b");
	struct MemoryMap *memmap = (struct MemoryMap *)((max_t)loader_argument->memmap_ptr);
	max_t kernel_end_address = loader_argument->total_kernel_area_end;
	debug::out::printf("kernel_end_address : 0x%X\n" , kernel_end_address);
	new_mboundary.start_address = align_address(kernel_end_address , 4096); // padding
	/* The "kstruct" is location of all sorts of kernel structures that requires static memory location.
	 * The kernel makes a space about 1MB large at the rear of kernel image for static structures.
	 * This code (haha) basically searches what "memmap" entry contains the kernel image and calculates whether
	 * the kstruct area is available to be created.
	
	for(int i = 0; i < loader_argument->memmap_count; i++) {
		max_t address = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
		max_t length = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
		// debug::out::printf("seg%d : 0x%X~0x%X\n" , i , address , address+length);
		if(kernel_end_address >= address && kernel_end_address <= address+length
		&& loader_argument->total_kernel_area_start >= address && loader_argument->total_kernel_area_start <= address+length) { // is kernel inside?
			// determine boundary
			// If segment cannot hold more than designated size, just use maximum size for the segment
			new_mboundary.end_address = MIN(address+length , new_mboundary.start_address+LOADER_ARGUMENT_LENGTH);
			debug::out::printf(DEBUG_INFO , "kstruct_boundary : 0x%X~0x%X\n" , new_mboundary.start_address , new_mboundary.end_address);
			debug::pop_function();
			return;
		}
	}
	new_mboundary.end_address = new_mboundary.start_address+LOADER_ARGUMENT_LENGTH;
	debug::out::printf(DEBUG_INFO , "def,kstruct_boundary : 0x%X~0x%X\n" , new_mboundary.start_address , new_mboundary.end_address);
	debug::pop_function();
	return;
}*/

static void sort_boundaries_list(struct memory::Boundary *boundaries , int count) {
	for(int i = 0; i < count-1; i++) {
		bool swapped = false;
		for(int j = 0; j < count-i-1; j++) {
			if(boundaries[j].start_address > boundaries[j+1].start_address) {
				struct memory::Boundary tmp = {boundaries[j].start_address , boundaries[j].end_address};
				boundaries[j].start_address = boundaries[j+1].start_address;
				boundaries[j].end_address   = boundaries[j+1].end_address;
				boundaries[j+1].start_address = tmp.start_address;
				boundaries[j+1].end_address   = tmp.end_address;
				swapped = true;
			}
		}
		if(swapped == false) break;
	}
}

/// @brief Merge the continunous boundaries into one
/// @param boundaries 
/// @param count 
/// @param merged_continuous_boundaries Merged version of boundaries
/// @return Numbers of items in new merged one 
int merge_boundaries_list(struct memory::Boundary *boundaries , int count , struct memory::Boundary *merged_continuous_boundaries) {
	struct memory::Boundary sorted_boundaries[count];
	memcpy(sorted_boundaries , boundaries , count*sizeof(struct memory::Boundary));
	sort_boundaries_list(sorted_boundaries , count);
	
	int THRESHOLD = 2048; // 2kB
    unsigned long continuous_segment_start = sorted_boundaries[0].start_address;
    int k;
    int actual_protected_area_count = 0;
	debug::out::printf("Sorted bounadry list : \n");
	for(int i = 0; i < count; i++) {
		debug::out::printf("0x%lx ~ 0x%lx\n" , sorted_boundaries[i].start_address , sorted_boundaries[i].end_address);
	}
	debug::out::printf("Merged segments : \n");
    for(k = 0; k < count-1; k++) {
        if(sorted_boundaries[k].end_address-sorted_boundaries[k+1].start_address > THRESHOLD) { // not continuous
            debug::out::printf("0x%lx ~ 0x%lx\n" , continuous_segment_start , sorted_boundaries[k].end_address);
            
            merged_continuous_boundaries[actual_protected_area_count].start_address = continuous_segment_start;
            merged_continuous_boundaries[actual_protected_area_count].end_address   = sorted_boundaries[k].end_address;
            actual_protected_area_count++;

            continuous_segment_start = sorted_boundaries[k+1].start_address;
        }
    }
    debug::out::printf("0x%lx ~ 0x%lx\n" , continuous_segment_start , sorted_boundaries[k].end_address);
    merged_continuous_boundaries[actual_protected_area_count].start_address = continuous_segment_start;
    merged_continuous_boundaries[actual_protected_area_count].end_address   = sorted_boundaries[k].end_address;
    actual_protected_area_count++;
    
	return actual_protected_area_count;
}

/// @brief Truncate the given protected areas from the memory map and 
//         create new boundary list(new_msegment_list) that completely excludes the protected boundary
/// @param protected_areas arrays of protected memory areas
/// @param protected_areas_count number of items on the array
/// @param new_msegment_list the list of new memory segment with protected areas removed
/// @param memmap the original memory map
/// @param memmap_count number of entries in the memory map
/// @return number of entries on new memory segment list
int truncate_protected_areas(struct memory::Boundary *protected_areas , int protected_areas_count , struct memory::Boundary *new_msegment_list , struct MemoryMap *memmap , int memmap_count) {
	int new_msegment_list_count = 0;
    for(int i = 0; i < memmap_count; i++) {
        if(memmap[i].type != MEMORYMAP_USABLE) continue;

        unsigned long memmap_start = (((unsigned long)memmap[i].addr_high << 32)|memmap[i].addr_low) , memmap_end = memmap_start+(((unsigned long)memmap[i].length_high << 32)|memmap[i].length_low);
        debug::out::printf("%d : 0x%lx~0x%lx\n" , i , memmap_start , memmap_end);

        unsigned long new_segment_addr = memmap_start;
        for(int j = 0; j < protected_areas_count; j++) {
            debug::out::printf("protected %d : 0x%lx ~ 0x%lx\n" , j , protected_areas[j].start_address , protected_areas[j].end_address);
            bool is_overlap_front = (memmap_start <= protected_areas[j].start_address && protected_areas[j].start_address <= memmap_end);
            bool is_overlap_rear  = (memmap_start <= protected_areas[j].end_address && protected_areas[j].end_address <= memmap_end);
            if(!is_overlap_front && !is_overlap_rear) continue;

            unsigned long overlap_region_start , overlap_region_end;
            if(is_overlap_front && !is_overlap_rear) { // only the front overlaps
                overlap_region_start = protected_areas[j].start_address;
                overlap_region_end   = memmap_end;
                debug::out::printf("1,");
            }
            if(!is_overlap_front && is_overlap_rear) {
                overlap_region_start = memmap_start;
                overlap_region_end   = protected_areas[j].end_address;
                debug::out::printf("2,");
            }
            if(is_overlap_front && is_overlap_rear) { // completely inside of the map entry
                overlap_region_start = protected_areas[j].start_address;
                overlap_region_end   = protected_areas[j].end_address;
                debug::out::printf("3,");
            }
            debug::out::raw_printf("memmap #%d, protected area #%d, overlapped area : 0x%lx~0x%lx\n" , i , j , overlap_region_start , overlap_region_end);
            if(new_segment_addr < overlap_region_start) {
                new_msegment_list[new_msegment_list_count].start_address = new_segment_addr;
                new_msegment_list[new_msegment_list_count].end_address = overlap_region_start;
                new_msegment_list_count++;
                debug::out::printf("     new segment : 0x%lx~0x%lx\n" , new_segment_addr , overlap_region_start);
            }
            new_segment_addr = overlap_region_end;
        }
        if(new_segment_addr < memmap_end) {
            new_msegment_list[new_msegment_list_count].start_address = new_segment_addr;
            new_msegment_list[new_msegment_list_count].end_address = memmap_end;
            new_msegment_list_count++;
            debug::out::printf("     new segment : 0x%lx~0x%lx\n" , new_segment_addr , memmap_end);
        }
    }
    return new_msegment_list_count;
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

max_t memory::kstruct_get_current_addr(void) { return kstruct_mgr.current_addr; }
void memory::kstruct_rollback_addr(max_t prev_addr) { if(kstruct_mgr.boundary.start_address <= prev_addr && prev_addr <= kstruct_mgr.boundary.end_address) { kstruct_mgr.current_addr = prev_addr; } }

void memory::SegmentsManager::init(int segment_count , struct Boundary *usable_segments) {
	managers_count = segment_count;
	total_memory = 0;
	// allocate space for all managers
	node_managers = (NodesManager *)kstruct_alloc(managers_count*sizeof(NodesManager));
	for(int i = 0; i < managers_count; i++) {
		node_managers[i].init(usable_segments[i].start_address , usable_segments[i].end_address);
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

void memory::pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct LoaderArgument *loader_argument) {
	debug::push_function("pmem_init");
	
	int usable_seg_count = 0;

	struct Boundary pmem_boundary[memmap_count*2];
	struct Boundary protected_areas[5] , merged_protected_areas[5];
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
	protected_areas[3].end_address   = loader_argument->memmap_location+align_address((loader_argument->memmap_count*sizeof(struct MemoryMap)) , 4096);
	// 5. kstruct Memory Area
	protected_areas[4].start_address = loader_argument->kstruct_mem_location;
	protected_areas[4].end_address   = loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size;

	// determine physical memory address
	memset(pmem_boundary , 0 , sizeof(pmem_boundary));
	int merged_protected_areas_count = merge_boundaries_list(protected_areas , 5 , merged_protected_areas);
	
	int pmem_boundary_entry_count = truncate_protected_areas(merged_protected_areas , merged_protected_areas_count , 
		pmem_boundary , memmap , memmap_count);
	debug::out::printf("Available memory area : \n");
	for(int i = 0; i < pmem_boundary_entry_count; i++) {
		debug::out::printf("0x%lx ~ 0x%lx\n" , pmem_boundary[i].start_address , pmem_boundary[i].end_address);
	}
	// allocate node manager for each segment
	SegmentsManager *segments_mgr = SegmentsManager::get_self();
	segments_mgr->init(pmem_boundary_entry_count , pmem_boundary);
	is_pmem_alloc_available = true;
	debug::pop_function();
}

max_t memory::SegmentsManager::get_currently_using_mem(void) {
	max_t currently_using_mem = 0;
	for(int i = 0; i < managers_count; i++) {
		currently_using_mem += node_managers[i].currently_using_mem;
		// debug::out::printf("node_managers[%d].currently_using_mem : %d\n" , i , node_managers[i].currently_using_mem);
	}
	return currently_using_mem;
}

vptr_t *memory::pmem_alloc(max_t size , max_t alignment) {
	vptr_t *ptr = 0x00;
	if(!is_pmem_alloc_available) return memory::kstruct_alloc(size , alignment);

	debug::push_function("pmem_alloc");
	SegmentsManager *segments_mgr = SegmentsManager::get_self();
	if(size == 0x00) { 
		debug::out::printf(DEBUG_WARNING , "allocation warning : zero allocation size\n");
		return 0x00;
	}
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
		debug::out::printf(DEBUG_WARNING , "Warning : Memory release request out of range(ptr=0x%lX)\n" , (max_t)ptr);
		// debug here
		debug::pop_function();
		return;
	}
	if(segments_mgr->node_managers[index].free((max_t)ptr) == false) {
		debug::out::printf(DEBUG_WARNING , "Warning : Memory release request not allocated(ptr=0x%lX)\n" , (max_t)ptr);
	}
	debug::pop_function();
}

bool memory::pmem_protect(struct Boundary boundary) {
	return false;
}

max_t memory::pmem_usage(void) { return GLOBAL_OBJECT(SegmentsManager)->get_currently_using_mem(); }