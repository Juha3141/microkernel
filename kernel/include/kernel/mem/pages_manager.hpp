/**
 * @file page_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel page allocator using bitmaps
 * @date 2025-01-29
 *
 * @copyright Copyright (c) 2024 Ian Juha Cho.
 *
 */

#ifndef _PAGES_MANAGER_HPP_
#define _PAGES_MANAGER_HPP_

#include <kernel/mem/kmem_manager.hpp>

#define PAGES_MANAGER_BITS_PER_PAGE 2
#define PAGES_MANAGER_BPP_MASK      0b11

namespace memory {
    class PagesManager {
        public:
            void init(max_t bitmap_start_address , max_t end_address , max_t page_size);
            
            max_t allocate(max_t number_of_pages);
            bool free(max_t ptr);

            bool available(void);

            max_t currently_using_mem;

            max_t bitmap_start_address;
            max_t bitmap_end_address;

            max_t mem_start_address;
            max_t mem_end_address;    
            max_t page_size;
        private:
            bool allocation_available;
    };
}

#endif