/**
 * @file integrated_drivers.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The header file that contains all the headers and lists of integrated drivers that will be used by the kernel.
 * @date 2024-02-27
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho. 
 * 
 */

#ifndef _INTEGRATED_DRIVERS_HPP_
#define _INTEGRATED_DRIVERS_HPP_

// headers here
#include <ramdisk/ramdisk.hpp>
#include <ide/ide.hpp>

// register_driver function here
#define INTEGRATED_DRIVER_LIST { \
    ramdisk_driver::register_driver , \
    ide_driver::register_driver , \
    ide_cd_driver::register_driver , \
}

#endif