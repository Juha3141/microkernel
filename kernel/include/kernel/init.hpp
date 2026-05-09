/**
 * @file init.hpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Collection of functions for initialization in kernel main function
 * @date 2026-05-09
 * 
 * @copyright Copyright (c) 2026 Ian Juha Cho
 * 
 */
#ifndef _INIT_HPP_
#define _INIT_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kasan.hpp>

struct MappedRegionInfo {
    max_t used_phys_addr_start;
    max_t used_phys_addr_end;
    max_t mapped_size;
    max_t page_size;
};

__no_sanitize_address__ MappedRegionInfo map_pages_auto_alignment(max_t phys_addr_start , max_t phys_addr_end , max_t linear_addr_start , max_t flags);
__no_sanitize_address__ void update_pt_space_to_kmemmap(void);

Boundary setup_kasan_shadowmem(max_t kernel_size , max_t kernel_stack_size);
Boundary map_usable_mem_into_vaddr(max_t linear_addr_start , max_t required_size , max_t flag=MEMORYMAP_USABLE);
max_t get_kernel_memory_pool_size();
max_t check_alignment(max_t address , int va_count , ...);

#endif