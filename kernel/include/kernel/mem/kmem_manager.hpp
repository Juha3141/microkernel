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

/// @brief Structure used for global kernel memory map
struct KernelMemoryMap {
	max_t start_address, end_address;
	unsigned int type;

    // used for linked list
    KernelMemoryMap *prev = 0x00, *next = 0x00;
};

/// @brief  Currently there's 5 essential kernel boundaries
///         These essential kernel memory boundaries are not covered by the loader argument's memory map, and
///         they are to be specially protected manually in kernel's code
///         1. Kernel Image
///         2. Kernel Stack
///         3. Loader Argument
///         4. Kernel Memory Map
///         5. kstruct Memory Area
#define essential_kernel_mem_boundaries(loader_argument) \
{                                                                                                                   \
    { /* Kernel Image */                                                                                            \
        .start_address = loader_argument->kernel_physical_location ,                                                \
        .end_address   = loader_argument->kernel_physical_location+loader_argument->kernel_size ,                   \
        .type          = MEMORYMAP_KERNEL_IMAGE                                                                     \
    } ,                                                                                                             \
    { /* Kernel Stack */                                                                                            \
        .start_address = loader_argument->kernel_stack_location ,                                                   \
        .end_address   = loader_argument->kernel_stack_location+loader_argument->kernel_stack_size ,                \
        .type          = MEMORYMAP_KERNEL_STACK                                                                     \
    } ,                                                                                                             \
    { /* Loader Argument */                                                                                         \
        .start_address = loader_argument->loader_argument_location ,                                                \
        .end_address   = loader_argument->loader_argument_location+loader_argument->loader_argument_size ,          \
        .type          = MEMORYMAP_LOADER_ARGUMENT                                                                  \
    } ,                                                                                                             \
    { /* Kernel Memory Map */                                                                                       \
        .start_address = loader_argument->memmap_location ,                                                         \
        .end_address   = loader_argument->memmap_location                                                           \
                +align_round_up((loader_argument->memmap_count*sizeof(LoaderMemoryMap)) , 4096) ,                   \
        .type          = MEMORYMAP_LOADER_ARGUMENT                                                                  \
    } ,                                                                                                             \
    { /* kstruct Memory Area */                                                                                     \
        .start_address = loader_argument->kstruct_mem_location ,                                                    \
        .end_address   = loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size ,                  \
        .type          = MEMORYMAP_KSTRUCT_POOL                                                                     \
    }                                                                                                               \
}

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
    void pmem_init(LoaderMemoryMap *memmap , max_t memmap_count , LoaderArgument *loader_argument);
    void *pmem_alloc(max_t size , max_t alignment=0);
    bool is_pmem_allocated_obj(void *ptr);
    void pmem_free(void *ptr);
    bool pmem_protect(struct Boundary boundary);

    max_t pmem_total_size(void);
    max_t pmem_usage(void);

    __no_sanitize_address__ void kmemmap_init(LoaderArgument *loader_argument);
    /// @brief global_kmemmap(from KernelMemmapManager) : Linked-list style global kernel memory map
    /// @return returns global kmemmap
    __no_sanitize_address__ KernelMemoryMap *&global_kmemmap();
    __no_sanitize_address__ void add_kmemmap_entry_overwrite(const KernelMemoryMap &entry);
    __no_sanitize_address__ void add_kmemmap_entry_overwrite(const LoaderMemoryMap &entry);
    __no_sanitize_address__ bool add_kmemmap_entry(const KernelMemoryMap& entry);
    __no_sanitize_address__ bool add_kmemmap_entry(const LoaderMemoryMap& entry);

    const char *memmap_type_to_str(unsigned int type);
}

#endif