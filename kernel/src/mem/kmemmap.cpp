/**
 * @file kmemmap.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel's central memory map manager. Helps adding/removing memory map entries
 * @date 2025-01-25
 * 
 */

#include <kernel/mem/kmem_manager.hpp>
#include <string.hpp>

// The global kernel memory map
KernelMemoryMap *kmemmap = nullptr;
KernelMemoryMap *kmemmap_discarded = nullptr;

/// @brief Add the memory map provided in the loader argument into the kmemmap (essentially convert it into linked list)
///        After which, add essential kernel memory boundaries, including but not limited to kernel image itself, kernel stack, or kstruct memory boundary.
/// @param loader_argument The loader argument
__no_sanitize_address__ void memory::kmemmap_init(LoaderArgument *loader_argument) {
	LoaderMemoryMap *loader_memmap = ((LoaderMemoryMap *)((max_t)loader_argument->memmap_location));
	KernelMemoryMap  essential_kmemmap[] = essential_kernel_mem_boundaries(loader_argument);

	for(int i = 0; i < loader_argument->memmap_count; i++) {
		max_t addr = ((max_t)loader_memmap[i].addr_high << 32)|loader_memmap[i].addr_low;
		max_t length = ((max_t)loader_memmap[i].length_high << 32)|loader_memmap[i].length_low;
		add_kmemmap_entry((KernelMemoryMap) {
			.start_address = addr , 
			.end_address   = addr+length , 
			.type = loader_memmap[i].type
		});
	}
	// essential kernel memory map (kernel image, stack, loader argument, kstruct memory, etc.)
	for(int i = 0; i < sizeof(essential_kmemmap)/sizeof(essential_kmemmap[0]); i++) {
		add_kmemmap_entry(essential_kmemmap[i]);
	}
	// video memory
	add_kmemmap_entry((KernelMemoryMap){
		.start_address = loader_argument->dbg_text_framebuffer_start ,
		.end_address   = loader_argument->dbg_text_framebuffer_end , 
		.type          = MEMORYMAP_VIDEOMEM
	});
	add_kmemmap_entry((KernelMemoryMap){
		.start_address = loader_argument->dbg_graphic_framebuffer_start , 
		.end_address   = loader_argument->dbg_graphic_framebuffer_end , 
		.type          = MEMORYMAP_VIDEOMEM
	});
}

__no_sanitize_address__ KernelMemoryMap *&memory::global_kmemmap() { return kmemmap; }

/// @brief allocate space for an entry for kernel memory map using kstruct_alloc
///        use the kmemmap_discarded pool if possible
///        * kmemmap_discarded : entries from kmemmap that are removed goes into the discarded pile, 
///          which is called kmemmap_discarded. The function kmemmap_entry_alloc() will automatically re-use 
///          the resource from the kmemmap_discarded in order to reduce the memory leakage.
///          
/// @return the new KernelMemoryMap that will be used in the kmemmap linked list
__no_sanitize_address__ static KernelMemoryMap *kmemmap_entry_alloc() {
    if(kmemmap_discarded == nullptr) return (KernelMemoryMap *)memory::kstruct_alloc(sizeof(KernelMemoryMap));

    KernelMemoryMap *new_one = kmemmap_discarded;
    kmemmap_discarded = kmemmap_discarded->next;
    return new_one;
}

__no_sanitize_address__ static KernelMemoryMap *kmemmap_entry_alloc(const KernelMemoryMap &entry) {
	KernelMemoryMap *new_entry = kmemmap_entry_alloc();
	memcpy(new_entry , &(entry) , sizeof(KernelMemoryMap));
	return new_entry;
}

/// @brief Doesn't actually remove the entry; instead, make start_address and end_address zero and stash the 
///        entry into the kmemmap_discarded.
/// @param ptr the entry that is to be removed
__no_sanitize_address__ static void kmemmap_entry_remove(KernelMemoryMap *ptr) {
	// detach the resource from the linked list
    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;

    ptr->prev = 0x00;

	// Add the resource into the ear of the kmemmap_discarded
    KernelMemoryMap *discarded_ptr = kmemmap_discarded , *discarded_ptr_prev = 0x00;
    while(discarded_ptr != nullptr) {
		discarded_ptr_prev = discarded_ptr;
        discarded_ptr = discarded_ptr->next;
    }
	discarded_ptr_prev->next = ptr;
	ptr->prev = discarded_ptr_prev;
	ptr->next = 0x00;
}

/// @brief insert the new_entry between "A" and "B" in the linked list
/// @return return the pointer to the newly created (or merged) entry
__no_sanitize_address__ static KernelMemoryMap *kmemmap_entry_insert_between(KernelMemoryMap *A , KernelMemoryMap *B , const KernelMemoryMap &entry) {
	if(A == nullptr) { // B is the kmemmap, update kmemmap
		if(B->start_address == entry.end_address && B->type == entry.type) {
			B->start_address = entry.start_address;
			return B;
		}

		KernelMemoryMap *new_entry = kmemmap_entry_alloc(entry);
		kmemmap = new_entry;
		new_entry->next = B;
		B->prev = kmemmap;
		return new_entry;
	}

	// don't create new entry but just merge into adjacent one, 
	// if the adjacent entry has same type and contiguous in terms of the address space
	if(A->end_address == entry.start_address && A->type == entry.type) {
		A->end_address = entry.end_address;
		return A;
	}
	if(B->start_address == entry.end_address && B->type == entry.type) {
		B->start_address = entry.start_address;
		return B;
	}

	KernelMemoryMap *new_entry = kmemmap_entry_alloc(entry);
	new_entry->next = A->next;
	new_entry->prev = A;

	A->next->prev = new_entry;
	A->next = new_entry;
	return new_entry;
}

/// @brief Helper function. Check whether ptr is inside entry, if it does, return true. Otherwise, return false
inline static bool inside(KernelMemoryMap *ptr , const KernelMemoryMap &entry) {
	return (entry.start_address <= ptr->start_address && ptr->start_address <= entry.end_address
		&& entry.start_address <= ptr->end_address   && ptr->end_address   <= entry.end_address);
}

/// @brief Helper function. Check whether overwriting entry into ptr is applicable by checking their type.
///        The condition for overwriting is either ptr must have same type with entry, or ptr is MEMORYMAP_USABLE
inline static bool overwrite_applicable(KernelMemoryMap *ptr , const KernelMemoryMap &entry) {
	return (ptr->type == entry.type)||(ptr->type == MEMORYMAP_USABLE);
}

/// @brief  
/// @param ptr 
/// @param entry 
/// @return 
__no_sanitize_address__ static bool kmemmap_entry_overwrite(KernelMemoryMap *ptr , const KernelMemoryMap &entry) {
	bool overlap_front = (entry.start_address > ptr->start_address && entry.start_address < ptr->end_address);
	bool overlap_back  = (entry.end_address   > ptr->start_address && entry.end_address   < ptr->end_address);
	if(!overlap_front && !overlap_back) return false;
	if(!overwrite_applicable(ptr , entry)) {
		debug::out::printf("(kmemmap_entry_overwrite) Warning : unsable to overwrite to kmemmap entry [0x%llx - 0x%llx, %d], which is not a free pool\n" , ptr->start_address , ptr->end_address , ptr->type);
		return false;
	}

	// Overwrite special case : entry is inside the ptr
    /* pS                 pE
	 * +-------------------+      pS    eS      eE             pE
	 * |        ptr        |       +-----+-------+--------------+
	 * +-----|-------|-----+   --> | ptr | entry | new_residual |
	 *       | entry |             +-----+-------+--------------+
	 *       +-------+          - ptr becomes  : pS - eS
	 *      eS      eE          - new_residual : eE - pE
	 * pS: ptr->start_address, pE: ptr->end_address, eS: entry.start_address, eE: entry.end_address
	 */
	if(overlap_front && overlap_back) {
		KernelMemoryMap new_residual;
		new_residual.start_address = entry.end_address;
		new_residual.end_address   = ptr->end_address;
		new_residual.type          = ptr->type;
 
		ptr->end_address = entry.start_address;

		KernelMemoryMap *new_entry = kmemmap_entry_insert_between(ptr , ptr->next , entry);
		kmemmap_entry_insert_between(new_entry , new_entry->next , new_residual);

		return true;
	}
	// if ptr is completely within the entry, remove the ptr
	if(inside(ptr , entry)) {
		kmemmap_entry_remove(ptr);
		return false;
	}

	if(overlap_front) { ptr->end_address = entry.start_address; }
	if(overlap_back) {
		ptr->start_address = entry.end_address; 

		kmemmap_entry_insert_between(ptr->prev , ptr , entry);
		return true;
	}
	return false;
}

/// @ Check whether 
/// @param entry 
/// @return 
__no_sanitize_address__ static bool check_overwriting_allowed(const KernelMemoryMap &entry) {
	KernelMemoryMap *ptr = kmemmap;
	while(ptr != nullptr) {
		// if ptr is exactly same, but the type is not overwrite-applicable --> return false
		if(ptr->start_address == entry.start_address && ptr->end_address == entry.end_address && !overwrite_applicable(ptr , entry)) return false;
		
		// if ptr is inside entry, or the entry's front is overlapping, or the back is overlapping,
		// but the overwriting is not applicable --> return false
		bool overlap_front = (entry.start_address > ptr->start_address && entry.start_address < ptr->end_address);
		bool overlap_back  = (entry.end_address   > ptr->start_address && entry.end_address   < ptr->end_address);
	
		if((inside(ptr , entry)||overlap_front||overlap_back) && !overwrite_applicable(ptr , entry)) return false; 
		ptr = ptr->next;
	}
	return true;
}

/// @brief Add the entry into the kmemmap linked list. The add_kmemmap_entry() will ONLY overwrite the already existing 
///        entry if and ONLY if the entry that will be overwritten has MEMORYMAP_USABLE type.
///        No overwritting will be occurred if the area that will be overwritten has the same type as the 
///        entry given in the argument.
/// @param entry 
/// @return Return true if it successfully added(or overwritten) the entry, false if failed
__no_sanitize_address__ bool memory::add_kmemmap_entry(const KernelMemoryMap &entry) {
	/* Upon initialization, variables might not have nullptr, unlike in app environment.
	 * (That is because contents in RAM is random in real life, not initialized to nullptr)
	 * Thus, for that reason, we use is_kstruct_allocated_obj to determine whether it's valid or not. 
	 */
	if(!is_kstruct_allocated_obj(kmemmap)) {
		kmemmap = kmemmap_entry_alloc(entry);
		kmemmap->next = nullptr;
		return true;
	}

	if(!check_overwriting_allowed(entry)) {
		debug::out::printf("Warning : overwriting not applicable for the entry [0x%llx ~ 0x%llx, %d]\n" , entry.start_address , entry.end_address , entry.type);
		return false;
	}
	
	KernelMemoryMap *ptr = kmemmap;
	while(ptr != nullptr) {
		// if there's an identical entry in the linked list, only overwrite the entry type 
		if(ptr->start_address == entry.start_address 
		&& ptr->end_address   == entry.end_address) {
			ptr->type = entry.type;
			return true;
		}

		if(kmemmap_entry_overwrite(ptr , entry)) return true;
		if(entry.end_address <= ptr->start_address) break;
		ptr = ptr->next;
	}
	
	return kmemmap_entry_insert_between(ptr->prev , ptr , entry);
}

const char *memory::memmap_type_to_str(unsigned int type) {
    switch(type) {
        case MEMORYMAP_USABLE: return "Usable";
        case MEMORYMAP_RESERVED: return "Reserved";
        case MEMORYMAP_ACPI_RECLAIM: return "ACPI Reclaimable";
        case MEMORYMAP_ACPI_NVS: return "ACPI NVS";
        case MEMORYMAP_UNUSABLE: return "Unusable";
        case MEMORYMAP_EFI_LOADER: return "EFI Loader";
        case MEMORYMAP_EFI_RUNTIME: return "EFI Runtime";
        case MEMORYMAP_EFI_BOOT_SERVICE: return "EFI Boot Service";
        case MEMORYMAP_VIDEOMEM: return "Video Memory";
        case MEMORYMAP_KERNEL_IMAGE: return "Kernel Image";
        case MEMORYMAP_KERNEL_STACK: return "Kernel Stack";
        case MEMORYMAP_LOADER_ARGUMENT: return "Loader Argument";
        case MEMORYMAP_KSTRUCT_POOL: return "Kstruct Pool";
        case MEMORYMAP_PT_SPACE: return "PT Space";
        case MEMORYMAP_MISCELLANEOUS: return "Miscellaneous";
    }
    return "Miscellaneous";
}