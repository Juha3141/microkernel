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
    void *alloc_pt_space(max_t size , max_t alignment);
    memory::Boundary get_pt_space_boundary(void);
    
    // In case we need to initialize the page table data before we use it
    void ARCHDEP init_page_table_data(PageTableData &page_table_data);
    /// @brief Set one page entry corresponding to the linear address/page size by provided physical address and flags
    /// @param page_table_data Architecture-dependent page table metadata 
    /// @param linear_address 
    /// @param page_size Size of one page
    /// @param physical_address Physical address that the corresponding page entry will be mapped
    /// @param flags Flags
    /// @param alloc_func The allocator that will be used to allocate memory space for page tables
    /// @return Returns true if successfully mapped, false if the page size is the size that's not supported
    bool ARCHDEP map_one_page(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
    
    /// @brief Register the page table into the system.
    /// @param page_table_data the page table data that will be registered
    void ARCHDEP register_page_table(PageTableData &page_table_data);

    // helps things
    bool map_pages(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
};

#endif