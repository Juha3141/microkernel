/**
 * @file architecture_type.hpp
 * @brief Provides basic type for operating system
 * @author Ian Juha Cho
 * Contact : ianisnumber2027@gmail.com
 */

#ifndef _ARCHITECTURE_TYPE_HPP_
#define _ARCHITECTURE_TYPE_HPP_

#define MEMMODEL_PHYSICAL_MAPPING 0x01 // 1:1 scale of physical memory and virtual memory
#define MEMMODEL_DEMAND_PAGING    0x02 // Demand Paging

#define ARCHITECTURE_LIMIT 0xFFFFFFFFFFFFFFFF
#define ARCHITECTURE_LIMIT_BIT 64

#define INVALID ARCHITECTURE_LIMIT

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

typedef void ptr_t;

typedef int default_t;

// Size of IO port
typedef unsigned char io_data;
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

#define MEMMAP_TYPE_USABLE     1
#define MEMMAP_TYPE_RESERVED   2
#define MEMMAP_TYPE_SYSTEM_ETC 3

/// @brief Describes one segment of memory
typedef struct {
    max_t start_address;
    max_t end_address;
    bool system_specific; // true  : "type" variable will be customized
                          // false : "type" variable will follow memory type from MEMMAP_TYPE
    byte type;   // indicates what type is this segment of memory
}memory_map;

#endif