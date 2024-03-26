/**
 * @file file_systems.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The header file that contains all the headers and lists of file systems that will be used by the kernel. 
 * @date 2024-02-27
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho. 
 * 
 */

#ifndef _FILE_SYSTEMS_HPP_
#define _FILE_SYSTEMS_HPP_

// headers here
#include <partitions/mbr.hpp>
#include <partitions/gpt.hpp>

#include <fat/fat12.hpp>
#include <fat/fat16.hpp>

// register_driver function here
#define FILESYSTEM_DRIVER_LIST { \
    MBRPartitionDriver::register_driver , \
    GPTPartitionDriver::register_driver , \
    fat12::register_driver , \
    fat16::register_driver , \
}

#endif