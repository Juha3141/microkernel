/**
 * @file kmem_manager.cpp
 * @brief Core system of kernel memory manager
 * 
 * @author Ian Juha Cho
 * contact : ianisnumber2027@gmail.com
 */

#include <kmem_manager.hpp>
#include <nodes_manager.hpp>
#include <string.hpp>
#include <debug.hpp>

// Just temporary patch
struct {
	struct memory::Boundary boundary;
	max_t current_addr;
}kstruct_mgr;

void memory::get_kstruct_boundary(struct Boundary &boundary) {
	memcpy(&boundary , &kstruct_mgr.boundary , sizeof(struct Boundary));
}

void memory::determine_kstruct_boundary(struct memory::Boundary &new_mboundary , struct KernelInfoStructure *kinfostruct) {
	debug::push_function("d_kstruct_b");
	struct MemoryMap *memmap = (struct MemoryMap *)kinfostruct->memmap_ptr;
	max_t kernel_end_address = kinfostruct->kernel_address+kinfostruct->kernel_size;
	debug::out::printf("kernel_end_address : 0x%X\n" , kernel_end_address);
	new_mboundary.start_address = align_address(kernel_end_address , 4096); // padding

	// unnecessary but necessary I guess
	for(int i = 0; i < kinfostruct->memmap_count; i++) {
		max_t address = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
		max_t length = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
		debug::out::printf("seg%d : 0x%X~0x%X\n" , i , address , address+length);
		if(kernel_end_address >= address && kernel_end_address <= address+length
		&& kinfostruct->kernel_address >= address && kinfostruct->kernel_address <= address+length) { // is kernel inside?
			// determine boundary
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

int memory::determine_pmem_boundary(struct Boundary protect , struct MemoryMap *new_msegment_list , struct KernelInfoStructure *kinfostruct) {
	debug::push_function("d_pmem_b");

	int seg_index = 0;
	struct MemoryMap *memmap = (struct MemoryMap *)kinfostruct->memmap_ptr;
	struct Boundary kstruct_boundary;
	get_kstruct_boundary(kstruct_boundary);
	// unnecessary but necessary I guess
	for(int i = 0; i < kinfostruct->memmap_count; i++) {
		max_t address = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
		max_t length = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
		// debug::out::printf("(d_pmem_b) seg%d : 0x%X~0x%X\n" , i , address , address+length);
		if(protect.start_address >= address && protect.start_address <= address+length && protect.start_address-address >= KERNEL_MEMORY_SEGMENT_THRESHOLD) {
			max_t new_addr = address;
			max_t new_length = protect.start_address-address;
			new_msegment_list[seg_index].addr_low = new_addr & 0xFFFFFFFF;
			new_msegment_list[seg_index].addr_high = new_addr >> 32;
			new_msegment_list[seg_index].length_low = new_length & 0xFFFFFFFF;
			new_msegment_list[seg_index].length_high = new_length >> 32;
			debug::out::printf(DEBUG_TEXT , "opt1 seg%d. 0x%X~0x%X\n" , seg_index , new_addr , new_addr+new_length);
			seg_index++;

		}
		if(protect.end_address >= address && protect.end_address <= address+length) {
			max_t new_addr = align_address(protect.end_address , 4096); // padding
			max_t new_length = address+length-protect.end_address;
			new_msegment_list[seg_index].addr_low = new_addr & 0xFFFFFFFF;
			new_msegment_list[seg_index].addr_high = new_addr >> 32;
			new_msegment_list[seg_index].length_low = new_length & 0xFFFFFFFF;
			new_msegment_list[seg_index].length_high = new_length >> 32;
			debug::out::printf(DEBUG_TEXT , "opt2 seg%d. 0x%X~0x%X\n" , seg_index , new_addr , new_addr+new_length);
			seg_index++;
		}
		else if(protect.start_address <= address+length) {
			debug::out::printf("opt3 seg%d. 0x%X~0x%X\n" , seg_index , ((max_t)memmap[i].addr_high << 32)|(memmap[i].addr_low) , (((max_t)memmap[i].addr_high << 32)|(memmap[i].addr_low))+(((max_t)memmap[i].length_high << 32)|(memmap[i].length_low)));
			memcpy(&new_msegment_list[seg_index++] , &memmap[i] , sizeof(struct MemoryMap));
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

void memory::pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct KernelInfoStructure *kinfostruct) {
	debug::push_function("pmem_init");
	
	max_t total_memory = 0;
	int usable_seg_count = 0;

	struct Boundary kstruct_boundary;
	struct MemoryMap pmem_boundary[memmap_count];
	int pmem_entry_count = 0;

	// determine kernel struct address
	determine_kstruct_boundary(kstruct_boundary , kinfostruct);
	// determine physical memory address
	if((pmem_entry_count = determine_pmem_boundary({kinfostruct->kernel_address , kstruct_boundary.end_address} , pmem_boundary , kinfostruct)) == 0) {
		debug::panic("kmem_manager.cpp" , 55 , "pmem_init() : zero detected usable memory\n");
	}    
	
	kstruct_init(kstruct_boundary); // initialize kernel structure area
	
	struct NodesManager *node_mgr = (struct NodesManager *)kstruct_alloc(memmap_count*sizeof(struct NodesManager));
	debug::out::printf(DEBUG_SPECIAL , "node_mgr : 0x%X\n" , (unsigned long)node_mgr);
	for(int i = 0; i < pmem_entry_count; i++) {
		max_t address = (max_t)pmem_boundary[i].addr_low|((max_t)pmem_boundary[i].addr_high << 32);
		max_t length = (max_t)pmem_boundary[i].length_low|((max_t)pmem_boundary[i].length_high << 32);
		total_memory += length;
		debug::out::printf("0x%X ~ 0x%X\n" , address , address+length);

	}
	debug::out::printf("Total memory : %dMB\n" , total_memory/1024/1024);
	debug::pop_function();
}