/**
 * @file types.hpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Provides basic interface type for operating system
 * @date 2023-08-20
 * 
 * @copyright Copyright (c) 2023 Ian Juha Cho
 * 
 */

#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <loader/loader_argument.hpp>
#include <kernel/configurations.hpp>

namespace debug{ namespace out { void printf(const char *string , ...); }}

#define SINGLETON_FUNCTION get_self

#define SINGLETON_PATTERN_KSTRUCT(type) \
static type *SINGLETON_FUNCTION(void) {\
    /* for whatever reason, if you initialize the static variable with zero, the variable somehow gets located in a weird address :/ */ \
    static type *p;\
    if(p == 0x00) { \
        p = (type *)memory::kstruct_alloc(sizeof(type));\
    }\
    return p;\
}

#define SINGLETON_PATTERN_PMEM(type) \
static type *SINGLETON_FUNCTION(void) {\
    static type *p;\
    if(p == 0x00) { \
        p = (type *)memory::pmem_alloc(sizeof(type));\
    } \
    return p;\
}

#define GLOBAL_OBJECT(type) type::SINGLETON_FUNCTION() 

// Maximum calculatable data size, corresponds to architecture(16-bit,32-bit, ...)
typedef unsigned long  max_t;
typedef signed long    max_s_t;

typedef unsigned long  qword;
typedef unsigned long  uint64_t;
typedef signed long    qword_s;
typedef signed long    int64_t;

typedef unsigned int   dword;
typedef unsigned int   uint32_t;
typedef signed int     dword_s;
typedef signed int     int32_t;
typedef unsigned short word;
typedef unsigned short uint16_t;
typedef signed short   word_s;
typedef signed short   int16_t;
typedef unsigned char  byte;
typedef unsigned char  uint8_t;
typedef signed char    byte_s;
typedef signed char    int8_t;

typedef max_t ptr_t;

typedef unsigned int size_t;

typedef void (*interrupt_handler_t)(struct Registers *regs);

#define ARCHDEP // indicates that the function is architecture-dependent

#define offsetof(s, m)   ((max_t)&(((s *)0)->m))
#define alignto(d , a)   ((d)+((a)-((d)%(a))))

#endif