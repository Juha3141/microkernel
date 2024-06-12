#ifndef _GENERAL_DEVICE_DRIVER_HPP_
#define _GENERAL_DEVICE_DRIVER_HPP_

#include <kernel/essentials.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <kernel/interrupt/interrupt.hpp>

#include <kernel/io_port.hpp>

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

#endif