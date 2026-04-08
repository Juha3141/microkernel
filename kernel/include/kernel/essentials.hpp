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

#define __no_sanitize_address__ __attribute__((no_sanitize("address")))

#endif