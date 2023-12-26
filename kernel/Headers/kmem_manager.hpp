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
     // SegmentsManager : Manager of segments, decide what segments to be used next
    class SegmentsManager {
        public:
            
        private:

    };
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
    int determine_pmem_boundary(struct Boundary protect , struct MemoryMap *new_mboundary , struct KernelInfoStructure *kinfostruct);

    void kstruct_init(struct Boundary boundary);
    void *kstruct_alloc(max_t size , max_t alignment=0);
    
    // pmem (physical memory) allocation
    void pmem_init(max_t memmap_count , struct MemoryMap *memmap , struct KernelInfoStructure *kinfostruct);
    void *pmem_alloc(max_t size , max_t alignment=0);
    void pmem_free(ptr_t *ptr);
}

#endif