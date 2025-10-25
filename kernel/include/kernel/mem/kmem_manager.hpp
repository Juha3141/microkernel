/**
 * @file kmem_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel Memory(Physical memory) Allocator
 * @date 2023-11-12
 */

#ifndef _KERNEL_MEMORY_MANAGER_H_
#define _KERNEL_MEMORY_MANAGER_H_

#include <kernel/essentials.hpp>
#include <kernel/mem/nodes_manager.hpp>

#include <loader/loader_argument.hpp>

#define KERNEL_MEMORY_SEGMENT_THRESHOLD 512*1024  // 512KB

namespace memory {
    typedef void *(*memory_allocator_func_t)(max_t size, max_t alignment);
    typedef void(*memory_deallocator_func_t)(void *ptr);

    struct Boundary {
        max_t start_address;
        max_t end_address;
    };
    void get_kstruct_boundary(struct Boundary &boundary);
    
    void kstruct_init(struct Boundary boundary);
    void *kstruct_alloc(max_t size , max_t alignment=0);
    bool is_kstruct_allocated_obj(void *obj);

    max_t kstruct_get_current_addr(void);
    void kstruct_rollback_addr(max_t prev_addr);
    
    // SegmentsManager : Manager of segments, decide what segments to be used next
    // Global class, use singleton pattern
    struct SegmentsManager {
        void init(int segment_count , Boundary *usable_segments);
        SINGLETON_PATTERN_KSTRUCT(SegmentsManager);
        
        int get_segment_index(max_t address);
        
        max_t get_currently_using_mem(void);
        max_t total_memory;
        int managers_count;
        NodesManager *node_managers;
    };
    
    // pmem (physical memory) allocation
    void pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct LoaderArgument *kargument);
    void *pmem_alloc(max_t size , max_t alignment=0);
    bool is_pmem_allocated_obj(void *ptr);
    void pmem_free(void *ptr);
    bool pmem_protect(struct Boundary boundary);

    max_t pmem_total_size(void);
    max_t pmem_usage(void);
}

#endif