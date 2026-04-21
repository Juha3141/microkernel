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

#ifdef CONFIG_USE_KASAN
#include <kernel/mem/kasan.hpp>
#endif

// for C++ standard

void *operator new(size_t size) { return memory::pmem_alloc(size); }
void *operator new[](size_t size) { return memory::pmem_alloc(size); }
void operator delete(void *ptr) { memory::pmem_free(ptr); }
void operator delete[](void *ptr) { memory::pmem_free(ptr); }

// Just temporary patch
struct {
	struct memory::Boundary boundary;
	max_t current_addr;
}kstruct_mgr;

bool is_pmem_alloc_available = false;

void memory::get_kstruct_boundary(struct Boundary &boundary) {
	memcpy(&boundary , &kstruct_mgr.boundary , sizeof(struct Boundary));
}

__no_sanitize_address__ void memory::kstruct_init(struct memory::Boundary kstruct_pmem_boundary) {
	max_t vmem_start_addr = PMEM_TO_VMEM(kstruct_pmem_boundary.start_address);
	max_t vmem_end_addr   = PMEM_TO_VMEM(kstruct_pmem_boundary.end_address);
	
	kstruct_mgr.boundary.start_address = vmem_start_addr;
	kstruct_mgr.boundary.end_address   = vmem_end_addr;
	kstruct_mgr.current_addr = vmem_start_addr;

	// initialize the memory
	memset((void *)vmem_start_addr , 0 , vmem_end_addr-vmem_start_addr);
}

__no_sanitize_address__ void *memory::kstruct_alloc(max_t size , max_t alignment) {
	max_t addr = align_round_up(kstruct_mgr.current_addr , alignment); // Align address
	kstruct_mgr.current_addr = addr+size; // increment address
	if(kstruct_mgr.current_addr >= kstruct_mgr.boundary.end_address) {
		debug::panic("kmem_manager.cpp" , 40 , "kstruct_alloc() : full kernel struct space\n");
	}
	return (void *)addr;
}

__no_sanitize_address__ bool memory::is_kstruct_allocated_obj(void *obj) {
	return kstruct_mgr.boundary.start_address <= (max_t)obj && (max_t)obj <= kstruct_mgr.current_addr;
}

max_t memory::kstruct_get_current_addr(void) { return kstruct_mgr.current_addr; }
void memory::kstruct_rollback_addr(max_t prev_addr) { if(kstruct_mgr.boundary.start_address <= prev_addr && prev_addr <= kstruct_mgr.boundary.end_address) { kstruct_mgr.current_addr = prev_addr; } }

void memory::SegmentsManager::init() {
	nodes_managers.init();
}

void memory::SegmentsManager::add_nodes_manager(const memory::Boundary &mem_boundary) {
	NodesManager new_node_mgr;
	new_node_mgr.init(mem_boundary.start_address , mem_boundary.end_address);
	nodes_managers.add_rear(new_node_mgr);
}

memory::NodesManager *memory::SegmentsManager::get_nodes_manager(max_t address) {
	auto *mgr = nodes_managers.search([&address](const NodesManager& mgr) {
		return (bool)((mgr.mem_start_address <= address) && (address <= mgr.mem_end_address));
	});
	if(mgr == nullptr) return nullptr;

	return &(mgr->object);
}

void memory::pmem_init() {
	SegmentsManager *segments_mgr = GLOBAL_OBJECT(SegmentsManager);
	segments_mgr->init();
	KernelMemoryMap *kmemmap_ptr = global_kmemmap();
	while(kmemmap_ptr != nullptr) {
		if(kmemmap_ptr->type != MEMORYMAP_USABLE) {
			kmemmap_ptr = kmemmap_ptr->next;
			continue;
		}
		max_t vmem_start_address = PMEM_TO_VMEM(kmemmap_ptr->start_address);
		max_t vmem_end_address   = PMEM_TO_VMEM(kmemmap_ptr->end_address);

		segments_mgr->add_nodes_manager({vmem_start_address , vmem_end_address});
		kmemmap_ptr = kmemmap_ptr->next;
	}
	is_pmem_alloc_available = true;
}

max_t memory::SegmentsManager::get_currently_using_mem(void) {
	max_t currently_using_mem = 0;
	auto *ptr = nodes_managers.get_start_node();
	while(ptr != nullptr) {
		debug::out::printf("%lld (0x%llx ~ 0x%llx)\n" , ptr->object.memory_usage , ptr->object.mem_start_address , ptr->object.mem_end_address);
		currently_using_mem += ptr->object.memory_usage;
		ptr = ptr->next;
	}
	return currently_using_mem;
}

static void *pmem_alloc_main(max_t size , max_t alignment) {
	memory::SegmentsManager *segments_mgr = memory::SegmentsManager::get_self();
	if(size == 0x00) return nullptr;
	void *ptr = nullptr;
	auto *node_s_ptr = segments_mgr->nodes_managers.get_start_node();

	while(node_s_ptr != nullptr) {
		if(node_s_ptr->object.available()) {
			if(ptr = (void *)node_s_ptr->object.allocate(size , alignment)) break;
		}

		node_s_ptr = node_s_ptr->next;
	}
	debug::out::printf("ptr return = 0x%llx\n" , ptr);
	return ptr;
}

static max_t pmem_free_main(void *ptr) {
	memory::NodesManager *node_mgr = GLOBAL_OBJECT(memory::SegmentsManager)->get_nodes_manager((max_t)ptr);
	if(node_mgr == nullptr) return 0;

	return node_mgr->free((max_t)ptr);
}

#ifdef CONFIG_USE_KASAN

__no_sanitize_address__
static void *kasan_pmem_alloc_hook(max_t size , max_t alignment) {
	void *ptr = nullptr;
	if(size == 0) return nullptr;
	if(alignment%KASAN_GRANUL_SIZE != 0) {
		debug::out::printf(DEBUG_WARNING , "Warning : Alignment should be a multiple of %d if KASan is enabled\n" , KASAN_GRANUL_SIZE);
		return nullptr;
	}
	
	max_t head_redzone_size = KASAN_HEAP_HEAD_REDZONE_SIZE;
	if(alignment == 0) alignment = KASAN_GRANUL_SIZE;
	else head_redzone_size = alignment;

	max_t aligned_sz = align_round_up(size , KASAN_GRANUL_SIZE);
	
	/* 
	 * <KASan redzone memory layout>
	 *               The address that will be       aligned by 
	 *               V    returned                  V   granul
	 * +-----------+-+-------------------------+----+-----------+
	 * |  redzone  |@|        allocated pool   |    redzone     |
	 * +-----------+-+-------------------------+----+-----------+
	 * ^            |<~~~~~~~~~~aligned_sz~~~~~~~~~~>
	 * |            |                          <---->
	 * aligned      |                         padding
	 * by granul    |
	 *              +--> Size of the redzone will be recorded right before the allocated pool
	 * 
	 * <------------------------------------------------------>
	 *     The size that the kasan hook will be requesting
	 *                     to the allocator
	 *  =  redzone_head 
	 *   + aligned_sz
	 *   + redzone_tail
	 * 
	 * 
	 * If the certain alignment is requested, the size of the redzone head will become
	 *  the size of alignment requested.
	 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 */

	// the actual allocation
	ptr = pmem_alloc_main(head_redzone_size+aligned_sz+KASAN_HEAP_TAIL_REDZONE_SIZE , alignment);
	max_t actual_ptr_start = (max_t)ptr+head_redzone_size;
	max_t kasan_head_start = (max_t)ptr;
	max_t kasan_head_size  = head_redzone_size;

	max_t kasan_tail_start = actual_ptr_start+size;
	max_t kasan_tail_size  = aligned_sz-size+KASAN_HEAP_TAIL_REDZONE_SIZE;

	// record the alignment
	*((max_t *)(actual_ptr_start-WORD_SIZE)) = head_redzone_size;

	kasan::poison_address(kasan_head_start , kasan_head_size , KASAN_SHADOW_MAGIC_HEAP_HEAD_REDZONE);
	kasan::poison_address(kasan_tail_start , kasan_tail_size , KASAN_SHADOW_MAGIC_HEAP_TAIL_REDZONE);
	kasan::unpoison_address(actual_ptr_start , aligned_sz);
	/*
	debug::out::printf(DEBUG_SPECIAL , "(alloc-POISON) kasan head redzone : 0x%llx~0x%llx\n" , kasan_head_start , kasan_head_start+kasan_head_size);
	debug::out::printf(DEBUG_SPECIAL , "(alloc-POISON) kasan tail redzone : 0x%llx~0x%llx\n" , kasan_tail_start , kasan_tail_start+kasan_tail_size);
	debug::out::printf(DEBUG_SPECIAL , "(alloc-UNPOISON) allocated memory pool : 0x%llx~0x%llx (sz=%lld)\n" , actual_ptr_start , actual_ptr_start+aligned_sz , aligned_sz);
	*/
	return (void *)actual_ptr_start;
}

__no_sanitize_address__
static void kasan_pmem_free_hook(void *ptr) {
	// to get the kasan head size
	max_t head_redzone_size = *(max_t *)((max_t)ptr-WORD_SIZE);
	
	max_t allocated_size = pmem_free_main((void *)((max_t)ptr-head_redzone_size));
	if(allocated_size == 0) return;
	
	max_t kasan_head_start = (max_t)ptr - head_redzone_size;
	max_t kasan_head_size  = head_redzone_size;
	/*
	 *                       |>------unpoison------<|
	 *                       <--- 8 ---> <--- 8 --->
	 * +-||---------------------+-------+-----------+
	 * | ||  allocated pool     | redz. |  redzone  |
	 * +-||---------------------+-------+-----------+
	 *                          <- ??? ->
	 *                          |
	 *                          +--> We don't know the exact alignment, 
	 *                               so just unpoison the 8 bytes, even though the 
	 *                               padding might not actually be 8 bytes.
	 * */

	max_t kasan_tail_start = (max_t)ptr+allocated_size-head_redzone_size-KASAN_HEAP_TAIL_REDZONE_SIZE+KASAN_GRANUL_SIZE;
	max_t kasan_tail_size  = KASAN_HEAP_TAIL_REDZONE_SIZE+KASAN_GRANUL_SIZE;

	// use-after-free protection area
	max_t uaf_start = (max_t)ptr;
	max_t uaf_size  = kasan_tail_start-(max_t)ptr;
	kasan::unpoison_address(kasan_head_start , kasan_head_size);
	kasan::unpoison_address(kasan_tail_start , kasan_tail_size);
	
	// Poison the de-allocated memory area to detect use-after-free leak
	kasan::poison_address(uaf_start , uaf_size , KASAN_SHADOW_MAGIC_HEAP_FREE);

	/*
	debug::out::printf(DEBUG_SPECIAL , "allocated size = %lld\n" , allocated_size);
	debug::out::printf(DEBUG_SPECIAL , "(free-UNPOISON) kasan head redzone : 0x%llx~0x%llx\n" , kasan_head_start , kasan_head_start+kasan_head_size);
	debug::out::printf(DEBUG_SPECIAL , "(free-UNPOISON) kasan tail redzone : 0x%llx~0x%llx\n" , kasan_tail_start , kasan_tail_start+kasan_tail_size);
	debug::out::printf(DEBUG_SPECIAL , "(use-after-free) 0x%llx ~ 0x%llx\n" , uaf_start , uaf_start+uaf_size);
	*/
}

#endif

void *memory::pmem_alloc(max_t size , max_t alignment) {
	if(!is_pmem_alloc_available) return memory::kstruct_alloc(size , alignment);
#ifdef CONFIG_USE_KASAN
	return kasan_pmem_alloc_hook(size , alignment);
#else
	return pmem_alloc_main(size , alignment);
#endif
}

void memory::pmem_free(void *ptr) {
#ifdef CONFIG_USE_KASAN
	kasan_pmem_free_hook(ptr);
#else
	pmem_free_main(ptr);
#endif
}

bool memory::is_pmem_allocated_obj(void *ptr) {
	NodesManager *nodes_mgr = GLOBAL_OBJECT(SegmentsManager)->get_nodes_manager((max_t)ptr);
	if(nodes_mgr == nullptr) return false;

	return nodes_mgr->is_allocated((max_t)ptr);
}

// not implemented
bool memory::pmem_protect(struct Boundary boundary) {
	return false;
}

max_t memory::pmem_usage(void) { return GLOBAL_OBJECT(SegmentsManager)->get_currently_using_mem(); }