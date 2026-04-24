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
#include <linked_list.hpp>

#include <loader/loader_argument.hpp>

#define KERNEL_MEMORY_SEGMENT_THRESHOLD 512*1024  // 512KB

/// @brief Structure used for global kernel memory map
struct KernelMemoryMap {
	max_t start_address, end_address;
	unsigned int type;

    // used for linked list
    KernelMemoryMap *prev, *next;
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
    { /* Loader Argument */                                                                                         \
        .start_address = loader_argument->ramdisk_location ,                                                        \
        .end_address   = loader_argument->ramdisk_location+loader_argument->ramdisk_size ,                          \
        .type          = MEMORYMAP_RAMDISK                                                                          \
    } ,                                                                                                             \
}

struct Boundary {
    max_t start_address;
    max_t end_address;
};

///////////////// kstruct /////////////////
namespace memory {
    void get_kstruct_boundary(Boundary& boundary);
    
    void kstruct_init(LoaderArgument *loader_argument);
    void *kstruct_alloc(max_t size , max_t alignment=0);
    bool is_kstruct_allocated_obj(void *obj);

    max_t kstruct_get_current_addr();

    template <typename T>T *new_global_object() {
        return (T *)kstruct_alloc(sizeof(T));
    }
}

///////////////// general physical memory allocator /////////////////
namespace memory {
    typedef void *(*memory_allocator_func_t)(max_t size, max_t alignment);
    typedef void(*memory_deallocator_func_t)(void *ptr);
    
    // pmem (physical memory) allocation
    void pmem_init();
    void *pmem_alloc(max_t size , max_t alignment=0);
    bool is_pmem_allocated_obj(void *ptr);
    void pmem_free(void *ptr);
    bool pmem_protect(Boundary boundary);

    max_t pmem_total_size();
    max_t pmem_usage();
    
    // SegmentsManager : Manager of segments, decide what segments to be used next
    // Global class, use singleton pattern
    struct SegmentsManager {
        void init();
        void add_nodes_manager(const Boundary &mem_boundary);
        
        NodesManager *get_nodes_manager(max_t address);
        
        max_t get_currently_using_mem(void);
        max_t total_memory;
        LinkedList<NodesManager>nodes_managers;
    };
}

////////////// kmemmap //////////////
namespace memory {
    void kmemmap_init(LoaderArgument *loader_argument);
    /// @brief global_kmemmap(from KernelMemmapManager) : Linked-list style global kernel memory map
    /// @return returns global kmemmap
    KernelMemoryMap *&global_kmemmap();
    KernelMemoryMap *add_kmemmap_entry(const KernelMemoryMap& entry);
    KernelMemoryMap *add_kmemmap_entry(const LoaderMemoryMap& entry);

    const char *memmap_type_to_str(unsigned int type);
}

#endif