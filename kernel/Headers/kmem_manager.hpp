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

namespace memory {
     // SegmentsManager : Manager of segments, decide what segments to be used next
    class SegmentsManager {
        public:
            
        private:

    };
    void init(void);
    void *kmem_alloc(max_t size , max_t alignment=0);
    void kmem_free(ptr_t *ptr);
    // Protect memory from being allocated by kernel.
}

#endif