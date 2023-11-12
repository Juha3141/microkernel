/**
 * @file nodes_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel memory nodes manager
 * @date 2023-11-12
 * 
 */

#ifndef _NODES_MANAGER_HPP_
#define _NODES_MANAGER_HPP_

#include <interface.hpp>
#include <interface_type.hpp>

#define MEMMANAGER_SIGNATURE 0x3141

namespace memory {
    struct Node {
        struct Node *next; // next node
        struct Node *previous; // previous node
        max_t size:63; // node size
        byte aligned:1;
        byte occupied:1;
        short signature:15;
    };
    // NodeManager : Manager of one big continuous segment
    class NodesManager {
        public:
            void init(max_t start_address , max_t total_usable_mem);

            max_t allocate(max_t size , max_t alignment);
            bool free(max_t ptr);
            
            struct Node *create_new_node(max_t size , max_t alignment);
            struct Node *align(struct Node *node , max_t alignment , max_t previous_node); // Align newly created node
            struct Node *search_first_fit(max_t size);
            struct Node *search_aligned(max_t size , max_t alignment);
            struct Node *search_new_node_location(max_t *prev_node);

            void write_node_data(struct Node *node , unsigned char occupied , max_t Size , max_t alignment , max_t next_node=INVALID , max_t previous_node=INVALID);

            bool available(void);
            
            static max_t align_address(max_t address , max_t alignment);
            static max_t get_node_size(struct Node *node);

            struct Node *node_start;
            max_t maximum_node_addr;
            max_t currently_using_mem;
        private:
            max_t mem_start_address;
            max_t mem_end_address;
            bool allocation_available;

            int unusable_memories_count;
            memory_map unusable_memories[512];
    };
}

#endif