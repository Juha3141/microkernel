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

#define PAGE_ENTRY_FLAGS_PRESENT   0x01
#define PAGE_ENTRY_FLAGS_KERNEL    0x02
#define PAGE_ENTRY_FLAGS_USER      0x04
#define PAGE_ENTRY_FLAGS_READ_ONLY 0x08
#define PAGE_ENTRY_FLAGS_RW        0x10
#define PAGE_ENTRY_FLAGS_EXD       0x20

namespace page {
    typedef void* (* func_alloc_pt_space_t)(max_t size , max_t alignment);
    void init_pt_space_allocator(LoaderArgument *loader_argument);
    void *alloc_pt_space(max_t size , max_t alignment);
    memory::Boundary get_pt_space_boundary(void);
    
    void init_higher_half(LoaderMemoryMap *memmap , max_t memmap_count , max_t higherhalf_relocation_addr);

    /// @brief Preemptively calculate the size of the identity page table, given the physical address end
    ///        Implement this function, so that the kernel can provide you with the page table location that can accomodate the page table size
    /// @param phys_addr_end 
    /// @return Size of the identity page table
    max_t calculate_identity_page_table_size(max_t phys_addr_end);
    
    void ARCHDEP map_one_page(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
    void ARCHDEP register_page_table(PageTableData &page_table_data);

    // helps things
    void map_pages(PageTableData &page_table_data , max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address , max_t flags
     , func_alloc_pt_space_t alloc_func);
};

#endif