/**
 * @file essentials.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief A header that includes essential kernel headers
 * @date 2024-06-12
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho.
 * 
 */

#ifndef _KERNEL_ESSENTIALS_HPP_
#define _KERNEL_ESSENTIALS_HPP_

#include <kernel/types.hpp>
#include <kernel/configurations.hpp>

#define min(X , Y) ((X) >= (Y) ? (Y) : (X))
#define max(X , Y) ((X) >= (Y) ? (X) : (Y))

constexpr max_t ARCH_MAXIMUM_PAGE_SIZE = 
#if CONFIG_USE_ENORMOUS_PAGE == yes
    CONFIG_ENORMOUS_PAGE_SIZE;
#elif CONFIG_USE_LARGE_PAGE == yes
    CONFIG_LARGE_PAGE_SIZE;
#else 
    CONFIG_PAGE_SIZE;
#endif

// Checks whether higher-half kernel is configured, and whether the address change is necessary.
// Used in TO_VMEM and TO_PMEM macro
extern "C" bool is_higherhalf_configured();

// If it's already in virtual memory space, don't change. It not, add/subtract the offset (and vice versa)
// Also, for TO_VMEM, check if higherhalf kernel is configured. If not, do not change the address.
#define TO_VMEM(addr) (((addr) < CONFIG_KERNEL_VMADDRESS) ? \
    (is_higherhalf_configured() ? ((addr)+CONFIG_KERNEL_VMADDRESS) : (addr)) \
    : (addr))
#define TO_PMEM(addr) (((addr) < CONFIG_KERNEL_VMADDRESS) ? (addr) : (addr-CONFIG_KERNEL_VMADDRESS))

#define __no_sanitize_address__ __attribute__((no_sanitize("address")))


#define CALL_FPTR_FROM_SECTION(section_start , section_end) for(qword func_ptr_ptr = (qword)&section_start; func_ptr_ptr < (qword)&section_end; func_ptr_ptr += sizeof(void *)) { \
        qword func_ptr = *((qword *)func_ptr_ptr); \
        ((void(*)(void))func_ptr)(); \
    }

#endif