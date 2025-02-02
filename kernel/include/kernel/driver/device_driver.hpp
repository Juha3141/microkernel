#ifndef _GENERAL_DEVICE_DRIVER_HPP_
#define _GENERAL_DEVICE_DRIVER_HPP_

#include <kernel/essentials.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/interrupt/interrupt.hpp>

#include <kernel/io_port.hpp>

typedef void(*driver_init_func_ptr_t)(void);

#define REGISTER_DRIVER_PRIORITY(init_driver , priority) void __register_driver_init_##priority_##init_driver(void) { init_driver(); } \
static __attribute__ ((section(".drivers_init."#priority))) driver_init_func_ptr_t __device_driver_init_##priority_##init_driver  = __register_driver_init_##priority_##init_driver; 

#define REGISTER_DRIVER(init_driver) REGISTER_DRIVER_PRIORITY(init_driver , 0)

typedef max_t resource_flag_t;
typedef max_t etc_resource_t;

struct device_resources {
    int io_port_count;
    io_port *io_ports;

    int interrupt_count;
    interrupt::interrupt_info_t *interrupts;
    
    int flags_count;
    resource_flag_t *flags;

    int etc_resources_count;
    etc_resource_t *etc_resources;
};

// Template : Device driver
template <typename T> struct general_device {
    max_t id;
    T *device_driver;
    device_resources resources;
};

// Template : Device container
template <typename T> struct general_device_driver {
    max_t driver_id;
    T *device_container;
    char driver_name[24];
};

/// @brief Create empty device with essential informations
/// @param driver driver for device
/// @param storage_type type of storage
/// @return new empty device
template <typename T> T *create_empty_device(void) {
    T *device = (T *)memory::pmem_alloc(sizeof(T));
    memset(device , 0 , sizeof(T));
    return device;
}

template <typename T> void designate_resources_count(T *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count) {
    if(io_port_count > 0) {
        device->resources.io_port_count = io_port_count;
        device->resources.io_ports = (io_port *)memory::pmem_alloc(io_port_count*sizeof(io_port));
    }
    if(interrupt_count > 0) {
        device->resources.interrupt_count = interrupt_count;
        device->resources.interrupts = (interrupt::interrupt_info_t *)memory::pmem_alloc(interrupt_count*sizeof(interrupt::interrupt_info_t));
    }
    if(flags_count > 0) {
        device->resources.flags_count = flags_count;
        device->resources.flags = (resource_flag_t *)memory::pmem_alloc(flags_count*sizeof(resource_flag_t));
    }
    if(etc_res_count > 0) {
        device->resources.etc_resources_count = etc_res_count;
        device->resources.etc_resources = (etc_resource_t *)memory::pmem_alloc(etc_res_count*sizeof(etc_resource_t));
    }
}

void register_kernel_drivers(void);

#endif