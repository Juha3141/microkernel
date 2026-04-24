/**
 * @file pages_manager.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel page manager
 * @date 2025-11-01
 * 
 */

#ifndef _PAGES_MANAGER_HPP_
#define _PAGES_MANAGER_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <page_table.hpp>

/// @brief If the kernel is configured to use large page, this value equals to CONFIG_LARGE_PAGE_SIZE.
///        Otherwise, it's equivalent to CONFIG_PAGE_SIZE.
constexpr int DEFAULT_PAGE_SIZE = 
#if CONFIG_USE_LARGE_PAGE == yes
            CONFIG_LARGE_PAGE_SIZE;
#else
            CONFIG_PAGE_SIZE;
#endif

#define PAGE_ENTRY_FLAGS_PRESENT   0x01
#define PAGE_ENTRY_FLAGS_KERNEL    0x02
#define PAGE_ENTRY_FLAGS_USER      0x04
#define PAGE_ENTRY_FLAGS_READ_ONLY 0x08
#define PAGE_ENTRY_FLAGS_RW        0x10
#define PAGE_ENTRY_FLAGS_EXD       0x20

namespace page {
    typedef void* (* func_alloc_pt_space_t)(max_t size , max_t alignment);
    bool init_pt_space_allocator(LoaderArgument *loader_argument);
    
    /// @brief Returns the physical/linear address of the global kernel page table
    ///        Both physical and linear address is identical, since the page table is identity-mapped
    max_t kernel_page_table();

    void *alloc_pt_space(max_t size , max_t alignment);
    Boundary get_pt_space_boundary(void);
    
    void higherhalf_is_now_configured();
    
    /// @brief Set one page entry corresponding to the linear address/page size by provided physical address and flags
    /// @param page_table_addr Page table address (Could be linear address or physical address)
    /// @param linear_address 
    /// @param page_size Size of one page
    /// @param physical_address Physical address that the corresponding page entry will be mapped
    /// @param flags Flags for the page table
    /// @param alloc_func The allocator that will be used to allocate memory space for page tables
    /// @return Returns true if successfully mapped, false if the page size is the size that's not supported
    bool ARCHDEP map_one_page(max_t page_table_addr , max_t linear_addr , max_t page_size , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
    
    /// @brief Register the page table into the system.
    /// @param page_table_addr Page table address
    void ARCHDEP register_page_table(max_t page_table_addr);

    /// @brief Helper function, calls map_one_page() by page_count times
    bool map_pages(max_t page_table_addr , max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
};

#endif