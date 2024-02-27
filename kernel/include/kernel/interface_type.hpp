/**
 * @file interface_type.hpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Provides basic interface type for operating system
 * @date 2023-08-20
 * 
 * @copyright Copyright (c) 2023 Ian Juha Cho
 * 
 */

#ifndef _INTERFACE_TYPE_HPP_
#define _INTERFACE_TYPE_HPP_

#include <kernel/kernel_argument.hpp>
#include <architecture_specification.hpp>

#define SINGLETON_FUNCTION get_self

#define SINGLETON_PATTERN_KSTRUCT(type) \
static type *SINGLETON_FUNCTION(void) {\
    static type *p = 0x00;\
    if(!p) p = (type *)memory::kstruct_alloc(sizeof(type));\
    return p;\
}

#define SINGLETON_PATTERN_PMEM(type) \
static type *SINGLETON_FUNCTION(void) {\
    static type *p = 0x00;\
    if(!p) p = (type *)memory::pmem_alloc(sizeof(type));\
    return p;\
}

#define GLOBAL_OBJECT(type) type::SINGLETON_FUNCTION() 

// Maximum calculatable data size, corresponds to architecture(16-bit,32-bit, ...)
typedef unsigned long max_t;
typedef signed long max_s_t;

typedef unsigned long qword;
typedef signed long qword_s;

typedef unsigned int dword;
typedef signed int dword_s;
typedef unsigned short word;
typedef signed short word_s;
typedef unsigned char byte;
typedef signed char byte_s;

typedef max_t ptr_t;
typedef void vptr_t;

typedef unsigned int size_t;

typedef int default_t;

// Size of IO port
typedef unsigned short io_port;

typedef void (*interrupt_handler_t)(void);

#endif