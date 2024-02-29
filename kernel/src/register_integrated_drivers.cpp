/**
 * @file register_integrated_drivers.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Consists of function that registers all the listed drivers to the kernel
 * @date 2024-02-27
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho. 
 * 
 */

#include <file_systems.hpp>
#include <integrated_drivers.hpp>

typedef void (*register_driver_func_t)(void);

void register_basic_kernel_drivers(void) {
    register_driver_func_t fs_drivers_list[] = FILESYSTEM_DRIVER_LIST; 
    register_driver_func_t int_drivers_list[] = INTEGRATED_DRIVER_LIST; 
    for(register_driver_func_t function : fs_drivers_list) { function(); } 
    for(register_driver_func_t function : int_drivers_list) { function(); } 
}