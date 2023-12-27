/**
 * @file kmem_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel Memory(Physical memory) Allocator
 * @date 2023-11-12
 */

#ifndef _KERNEL_MEMORY_MANAGER_H_
#define _KERNEL_MEMORY_MANAGER_H_

#include <interface.hpp>
#include <interface_type.hpp>
#include <nodes_manager.hpp>

#include <kernel_argument.hpp>

#define KERNEL_MEMORY_SEGMENT_THRESHOLD 512*1024  // 512KB

namespace memory {
    /*
    class MemoryMapManager {
        public:
            void init(void);
            void add_memory_map(const char *component , struct MemoryMap *memmap);
            void 
    };
    */
    struct Boundary {
        max_t start_address;
        max_t end_address;
    };
    void get_kstruct_boundary(struct Boundary &boundary);

    void determine_kstruct_boundary(struct Boundary &new_mboundary , struct KernelInfoStructure *kinfostruct);
    int determine_pmem_boundary(struct Boundary protect , struct Boundary *new_mboundary , struct KernelInfoStructure *kinfostruct);

    void kstruct_init(struct Boundary boundary);
    void *kstruct_alloc(max_t size , max_t alignment=0);
    
    // SegmentsManager : Manager of segments, decide what segments to be used next
    // Global class, use singleton pattern
    struct SegmentsManager {
        void init(int segment_count , struct Boundary *usable_segments);
        static SegmentsManager *get_self(void) {
            static SegmentsManager *ptr = 0x00;
            if(ptr == 0x00) {
                ptr = (SegmentsManager *)kstruct_alloc(sizeof(SegmentsManager));
            }
            return ptr;
        }
        int get_segment_index(max_t address);
        
        max_t get_currently_using_mem(void);
        max_t total_memory;
        int managers_count;
        NodesManager *node_managers;
    };
    
    // pmem (physical memory) allocation
    void pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct KernelInfoStructure *kinfostruct);
    ptr_t *pmem_alloc(max_t size , max_t alignment=0);
    void pmem_free(ptr_t *ptr);

    max_t pmem_total_size(void);
    max_t pmem_usage(void);
}

#endif