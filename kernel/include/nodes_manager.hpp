/**
 * @file nodes_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel memory nodes manager
 * @date 2023-11-12
 * 
 */

#ifndef _NODES_MANAGER_HPP_
#define _NODES_MANAGER_HPP_

#include <interface_type.hpp>

#define MEMMANAGER_SIGNATURE 0x3141

namespace memory {
    struct Node {
        struct Node *next; // next node
        struct Node *previous; // previous node
        qword size:63; // node size
        byte aligned:1;
        byte occupied:1;
        word signature:15;
    };
    max_t align_address(max_t address , max_t alignment);
    // NodeManager : Manager of one big continuous segment
    class NodesManager {
        public:
            void init(max_t start_address , max_t total_usable_mem);

            max_t allocate(max_t size , max_t alignment);
            bool free(max_t ptr);

            bool available(void);

            max_t currently_using_mem;

            max_t mem_start_address;
            max_t mem_end_address;
        private:
            struct Node *create_new_node(max_t size , max_t alignment);
            struct Node *align(struct Node *node , max_t alignment , max_t previous_node); // Align newly created node
            struct Node *search_first_fit(max_t size);
            struct Node *search_aligned(max_t size , max_t alignment);
            struct Node *search_new_node_location(max_t *prev_node);

            void write_node_data(struct Node *node , unsigned char occupied , max_t size , max_t alignment , max_t next_node=INVALID , max_t previous_node=INVALID);
            
            static max_t get_node_size(struct Node *node);

            struct Node *node_start;
            max_t maximum_node_addr;

            bool allocation_available;
    };
}

#endif