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

#define MIN(X , Y) ((X) >= (Y) ? (Y) : (X))
#define MAX(X , Y) ((X) >= (Y) ? (X) : (Y))

#define __no_sanitize_address__ __attribute__((no_sanitize("address")))

#endif