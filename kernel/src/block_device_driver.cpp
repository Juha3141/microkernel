#include <kernel/block_device_driver.hpp>
#include <linked_list.hpp>
#include <kernel/storage_system.hpp>

#include <kernel/kmem_manager.hpp>

#include <kernel/debug.hpp>

void blockdev::init(void) {
    // BlockDeviceDriverContainer : global container for kernel
    GLOBAL_OBJECT(BlockDeviceDriverContainer)->init(512);
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
max_t blockdev::register_driver(blockdev::block_device_driver *driver , const char *driver_name) {
    debug::push_function("bdev::reg_drv");

    BlockDeviceDriverContainer *driver_container = GLOBAL_OBJECT(BlockDeviceDriverContainer);
    max_t id = driver_container->register_object(driver); // register driver to global container
    driver->driver_id = id;
    if(id == INVALID) { debug::pop_function(); return INVALID; }
    driver->device_container = new BlockDeviceContainer;
    driver->device_container->init(256);
    // assign new local device container
    // Driver contains its devices
    strcpy(driver->driver_name , driver_name);
    driver->prepare();
    
    debug::out::printf("Registered device driver, id : %d name : \"%s\"\n" , driver->driver_id , driver->driver_name);
    debug::pop_function();
    return driver->driver_id;
}

///@brief some bridge-like functions(just basic stuff)

blockdev::block_device_driver *blockdev::search_driver(const char *driver_name) { 
    max_t id = GLOBAL_OBJECT(BlockDeviceDriverContainer)->search<const char *>([](block_device_driver *driver , const char *name) { return (bool)(strcmp(driver->driver_name , name) == 0); } , driver_name);  
    return GLOBAL_OBJECT(BlockDeviceDriverContainer)->get_object(id);
}

blockdev::block_device_driver *blockdev::search_driver(max_t driver_id) { return GLOBAL_OBJECT(BlockDeviceDriverContainer)->get_object(driver_id); }

max_t blockdev::discard_driver(const char *driver_name) { return GLOBAL_OBJECT(BlockDeviceDriverContainer)->discard_object(search_driver(driver_name)); }
max_t blockdev::discard_driver(max_t driver_id) { return GLOBAL_OBJECT(BlockDeviceDriverContainer)->discard_object(GLOBAL_OBJECT(BlockDeviceDriverContainer)->get_object(driver_id)); }

/// @brief Registeres device to driver (kernel)
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t blockdev::register_device(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    BlockDeviceDriverContainer *driver_container = BlockDeviceDriverContainer::get_self();

    if(driver->get_geometry(device , device->geometry) == false) return INVALID;
    device->id = driver->device_container->register_object(device);
    if(device->id == INVALID) { debug::out::printf(DEBUG_ERROR , "invalid id!\n"); return INVALID; }
    
    device->device_driver = driver;
    storage_system::detect_partitions(device);
    return device->id;
}

/// @brief Other form of register_device

max_t blockdev::register_device(const char *driver_name , blockdev::block_device *device) { return blockdev::register_device(search_driver(driver_name) , device); }
max_t blockdev::register_device(max_t driver_id , blockdev::block_device *device) { return blockdev::register_device(GLOBAL_OBJECT(BlockDeviceDriverContainer)->get_object(driver_id) , device); }

blockdev::block_device *blockdev::search_device(max_t driver_id , max_t device_id) { return search_driver(driver_id)->device_container->get_object(device_id); }
blockdev::block_device *blockdev::search_device(blockdev::block_device_driver *driver , max_t device_id) {  return driver->device_container->get_object(device_id); }
blockdev::block_device *blockdev::search_device(const char *driver_name , max_t device_id) { return blockdev::search_device(search_driver(driver_name) , device_id); }

bool blockdev::discard_device(blockdev::block_device *device) { return device->device_driver->device_container->discard_object(device); }

/// @brief Create empty device with essential informations
/// @param driver driver for device
/// @param storage_type type of storage
/// @return new empty device
blockdev::block_device *blockdev::create_empty_device(void) {
    block_device *device = (block_device *)memory::pmem_alloc(sizeof(block_device));
    memset(device , 0 , sizeof(block_device));
    return device;
}

template <typename T> inline T *alloc_by_cnt(int cnt) { return (T *)memory::pmem_alloc(cnt*sizeof(T)); } 

void blockdev::designate_resources_count(blockdev::block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count) {
    if(io_port_count > 0) {
        device->resources.io_port_count = io_port_count;
        device->resources.io_ports = alloc_by_cnt<io_port>(io_port_count);
    }
    if(flags_count > 0) {
        device->resources.flags_count = flags_count;
        device->resources.flags = alloc_by_cnt<resource_flag_t>(flags_count);
    }
    if(etc_res_count > 0) {
        device->resources.etc_resources_count = etc_res_count;
        device->resources.etc_resources = alloc_by_cnt<etc_resource_t>(etc_res_count);
    }
}