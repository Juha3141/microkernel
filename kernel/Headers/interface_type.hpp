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

#include <kernel_argument.hpp>

#define MEMMODEL_PHYSICAL_MAPPING 0x01 // 1:1 scale of physical memory and virtual memory
#define MEMMODEL_DEMAND_PAGING    0x02 // Demand Paging

#define ARCHITECTURE_LIMIT 0xFFFFFFFFFFFFFFFF
#define ARCHITECTURE_LIMIT_BIT 64

#define INVALID ARCHITECTURE_LIMIT

#define SINGLETON_PATTERN_KSTRUCT(type) \
static type *get_self(void) {\
    static type *p = 0x00;\
    if(!p) p = (type *)memory::kstruct_alloc(sizeof(type));\
    return p;\
}

#define SINGLETON_PATTERN_PMEM(type) \
static type *get_self(void) {\
    static type *p = 0x00;\
    if(!p) p = (type *)memory::pmem_alloc(sizeof(type));\
    return p;\
}

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

// Size of interrupt handler pointer (function pointer)
typedef unsigned long interrupt_handler;

// Describes information of one core in system
typedef struct {
    // To-do : Finish up the field //
    dword id;
}core;

/// @brief Describes general information of cores in system
typedef struct {
    dword total_core_cnt;
    dword physical_core_cnt;
    char vendor_id[12];

    char brand_string[48];
    qword CPU_speed;
}general_cpu_info;

#endif