///////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.hpp"                                               //
// Written by   : Juha Cho                                                   //
// Started Date : 2022.10.18                                                 //
// Description  : Header file of MemoryManagement.hpp, contains structure of //
// Memory Management System and allocation & disallocation function.         //
///////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORYMANAGEMENT_H_
#define _MEMORYMANAGEMENT_H_

#include <architecture.hpp>
#include <architecture_type.hpp>

#define MEMMANAGER_SIGNATURE 0x3141

namespace memory {
    struct Node {
        struct Node *next;
        struct Node *previous;
        max_t size:63;
        byte aligned:1;
        byte occupied:1;
        short signature:15;
    }; // Total 18 bytes
    class NodeManager {
        public:
            void init(max_t start_address , max_t total_usable_mem , int memmap_count , memory_map *memmap);
            struct Node *search_first_fit(max_t size);
            struct Node *search_aligned(max_t size , max_t alignment);
            struct Node *search_new_node_location(max_t *prev_node);
            
            struct Node *create_new_node(max_t size , max_t alignment);
            void write_node_data(struct Node *node , unsigned char occupied , max_t Size , max_t alignment , max_t next_node=INVALID , max_t previous_node=INVALID);
            
            struct Node *align(struct Node *node , max_t alignment , max_t previous_node); // Align newly created node
            /*
            int IsNodeInUnusableMemory(struct Node *node , memory_map *violated);
            void AddUnusableMemory(max_t start_address , max_t size);
            */
            
            struct Node *node_start;
            struct Node *current_node; // Next address of lastly allocated node
            struct Node *last_freed_node;
            max_t total_usable_memory;
            max_t currently_using_mem;
        private:
            int unusable_memories_count;
            memory_map unusable_memories[512];
    };
    void init(void);

    max_t align_address(max_t address , max_t alignment);
    
    max_t get_node_size(struct Node *node);
    void *kmem_alloc(max_t size , max_t alignment=0);
    void kmem_free(ptr_t *ptr);
    // Protect memory from being allocated by kernel.
}

#endif