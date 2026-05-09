#ifndef _GENERAL_DEVICE_DRIVER_HPP_
#define _GENERAL_DEVICE_DRIVER_HPP_

#include <kernel/essentials.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/interrupt/interrupt.hpp>

#include <kernel/io_port.hpp>

typedef void(*driver_init_func_ptr_t)(void);

#define REGISTER_FPTR_TO_SECTION(init_driver , section_name) void __register_driver_init_##init_driver(void) { init_driver(); } \
__attribute__ ((used)) __attribute__ ((section(section_name))) driver_init_func_ptr_t __device_driver_init_##init_driver  = __register_driver_init_##init_driver; 

#define REGISTER_DRIVER(init_driver) REGISTER_FPTR_TO_SECTION(init_driver , ".drivers_init")
#define REGISTER_FS_DRIVER(init_driver) REGISTER_FPTR_TO_SECTION(init_driver , ".fs_drivers_init")

typedef max_t resource_flag_t;
typedef max_t etc_resource_t;

enum driver_type_t {
    block, character
};

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


struct general_device;

// Template : Device container
struct device_driver {
    virtual bool prepare(void) = 0;
    virtual bool io_read(general_device *device , max_t command , max_t argument , max_t &data_out) = 0;
    virtual bool io_write(general_device *device , max_t command , max_t argument) = 0;
    
    max_t driver_id;
    FixedArray<general_device*> *device_container;
    char driver_name[24];

    driver_type_t type;
};

// When inheriting the device class, add the driver field, the pointer to the device's driver
struct general_device {
    max_t id;
    device_resources resources;

    device_driver *driver;
};

namespace dev {
    void init();
    void register_kernel_drivers(void);
    void register_file_system_drivers(void);

    template <typename T>
    max_t register_driver(T *driver , const char *driver_name , const driver_type_t &type);
    device_driver *search_driver(const char *driver_name);
    device_driver *search_driver(max_t driver_id);

    max_t discard_driver(const char *driver_name);
    max_t discard_driver(max_t driver_id);
    
    template <typename T>
    max_t register_device(T *driver , general_device *device);
    max_t register_device(const char *driver_name , general_device *device);
    max_t register_device(max_t driver_id , general_device *device);

    template <typename T>
    general_device *search_device(T *driver , max_t device_id);
    general_device *search_device(const char *driver_name , max_t device_id);
    general_device *search_device(max_t driver_id , max_t device_id);

    bool discard_device(general_device *device);
}

//////////////// Template Function Implementations ////////////////

extern FixedArray<device_driver *> *device_driver_container;

/// @brief Create empty device with essential informations
/// @param T device driver type(general_device/block_device/char_device)
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

template <typename T>
general_device *dev::search_device(T *driver , max_t device_id) {
    return driver->device_container->get(device_id);
}

template <typename T>
max_t dev::register_device(T *driver , general_device *device) {
    device->id = driver->device_container->add(device);
    if(device->id == INVALID) { debug::out::printf(DEBUG_ERROR , "invalid id!\n"); return INVALID; }
    
    device->driver = driver;
    return device->id;
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
template <typename T>
max_t dev::register_driver(T *driver , const char *driver_name , const driver_type_t &type) {
    max_t id = device_driver_container->add(driver); // register driver to global container
    driver->driver_id = id;
    driver->type      = type;
    strncpy(driver->driver_name , driver_name , 24);
    if(id == INVALID) { return INVALID; }
    
    driver->device_container = new FixedArray<general_device *>();
    driver->device_container->init(256);
    // assign new local device container
    // Driver contains its devices
    driver->prepare();
    
    debug::out::printf("Registered device driver, id : %d name : \"%s\"\n" , driver->driver_id , driver->driver_name);
    return driver->driver_id;
}

#endif