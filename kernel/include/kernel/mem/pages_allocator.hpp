/**
 * @file page_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel page allocator using bitmaps
 * @date 2025-01-29
 *
 * @copyright Copyright (c) 2025 Ian Juha Cho.
 *
 */

#ifndef _PAGES_ALLOCATOR_HPP_
#define _PAGES_ALLOCATOR_HPP_

#include <kernel/mem/kmem_manager.hpp>

#define PAGES_MANAGER_BITS_PER_PAGE 2
#define PAGES_MANAGER_BITS_MASK     0b11
#define PAGES_MANAGER_IDENTIFIER_BLOCK    0b11

namespace memory {
    class PagesManager {
        friend class NodesManager;
        public:
            void init(max_t bitmap_start_address , max_t end_address , max_t page_size);
            bool block_pages(max_t page_start_addr , max_t page_end_addr);
            bool unblock_pages(max_t start_page_number , max_t blocked_page_count);

            typedef enum {
                mem_free = 0 , mem_partially_occupied = 1 , mem_occupied = 2
            }memory_availability; 
            memory_availability check_availability(max_t start_address , max_t end_address);
            
            max_t allocate(max_t number_of_pages);
            bool free(max_t ptr , bool free_blocked_area=false , max_t blocked_area_size=0);

            bool available(void);

            max_t currently_using_mem;

            max_t bitmap_start_address;
            max_t bitmap_end_address;

            max_t mem_start_address;
            max_t mem_end_address;    
            max_t page_size;

            max_t memory_usage;
        private:
            bool allocation_available;
    };
}

#endif