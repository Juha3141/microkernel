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

namespace Memory {
    struct Node {
        struct Node *next;
        struct Node *previous;
        max_t size:63;
        byte aligned:1;
        byte occupied:1;
        short signature:15;
    }; // Total 18 bytes
    enum Alignment {
        NO_ALIGN = 0 , 
        ALIGN_4K = 4096 , 
        ALIGN_8K = 8192
    };
    class NodeManager {
        public:
            void Initialize(max_t start_address , max_t total_usable_mem , int memmap_count , memory_map *memmap);
            struct Node *SearchReasonableNode(max_t size);
            struct Node *SearchAlignedNode(max_t size , Alignment alignment);
            struct Node *SearchNewNodeLocation(max_t *prev_node);
            
            struct Node *CreateNewNode(max_t size , Alignment alignment);
            void WriteNodeData(struct Node *node , unsigned char occupied , max_t Size , Alignment alignment , max_t next_node=INVALID , max_t previous_node=INVALID);
            
            struct Node *AdjustNode(struct Node *node); // If node violated reserved memory, adjust it.
            struct Node *AlignNode(struct Node *node , Alignment alignment , max_t previous_node); // Align newly created node
            
            int IsNodeInUnusableMemory(struct Node *node , memory_map *violated);
            void AddUnusableMemory(max_t start_address , max_t size);
            
            void MapNodes(void);
            
            struct Node *node_start;
            struct Node *current_node; // Next address of lastly allocated node
            struct Node *last_freed_node;
            max_t total_usable_memory;
            max_t currently_using_mem;
        private:
            int unusable_memories_count;
            memory_map unusable_memories[512];
    };
    void Initialize(void);

    max_t AlignAddress(max_t address , Alignment alignment);
    
    max_t GetNodeSize(struct Node *node);
    bool IsMemoryInside(max_t source , max_t s_length , max_t target , max_t t_length);
    void *Allocate(max_t size , Alignment alignment=NO_ALIGN);
    void Free(ptr_t *ptr);
    // Protect memory from being allocated by kernel.
    void ProtectMemory(max_t start_address , max_t size);
}

#endif