#include <drivers/block_device_driver.hpp>
#include <linked_list.hpp>

#include <kmem_manager.hpp>

#include <debug.hpp>

void blockdev::init(void) {
    // BlockDeviceDriverContainer : global container for kernel
    BlockDeviceDriverContainer::get_self()->init(512);
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
max_t blockdev::register_driver(blockdev::block_device_driver *driver , const char *driver_name) {
    debug::push_function("bdev::reg_drv");

    BlockDeviceDriverContainer *driver_container = BlockDeviceDriverContainer::get_self();
    max_t id = driver_container->register_driver(driver); // register driver to global container
    if(id == INVALID) { debug::pop_function(); return INVALID; }
    
    // assign new local device container
    // Driver contains its devices
    strcpy(driver->driver_info.driver_name , driver_name);
    driver->prepare();
    
    debug::out::printf("Registered device driver, id : %d name : \"%s\"\n" , driver->driver_info.driver_id , driver->driver_info.driver_name);
    debug::pop_function();
    return driver->driver_info.driver_id;
}

///@brief some bridge-like functions(just basic stuff)

blockdev::block_device_driver *blockdev::search_driver(const char *driver_name) { return BlockDeviceDriverContainer::get_self()->search_by_name(driver_name); }
blockdev::block_device_driver *blockdev::search_driver(max_t driver_id) { return BlockDeviceDriverContainer::get_self()->get_object(driver_id); }

max_t blockdev::discard_driver(const char *driver_name) { return BlockDeviceDriverContainer::get_self()->discard_object(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name)); }
max_t blockdev::discard_driver(max_t driver_id) { return BlockDeviceDriverContainer::get_self()->discard_object(BlockDeviceDriverContainer::get_self()->get_object(driver_id)); }

/// @brief Registeres device to driver (kernel)
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t blockdev::register_device(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    debug::push_function("blockdev::reg_dev");
    BlockDeviceDriverContainer *driver_container = BlockDeviceDriverContainer::get_self();
    
    device->id = driver->device_container->register_object(device);
    if(device->id == INVALID) {
        return false;
    }
    device->device_driver = driver;
    return device->id;
}

/// @brief Other form of register_device

max_t blockdev::register_device(const char *driver_name , blockdev::block_device *device) { return blockdev::register_device(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name) , device); }
max_t blockdev::register_device(max_t driver_id , blockdev::block_device *device) { return blockdev::register_device(BlockDeviceDriverContainer::get_self()->get_object(driver_id) , device); }

blockdev::block_device *blockdev::search_device(max_t driver_id , max_t device_id) { return search_driver(driver_id)->device_container->get_object(device_id); }
blockdev::block_device *blockdev::search_device(blockdev::block_device_driver *driver , max_t device_id) {  return search_device(driver->driver_info.driver_id , device_id); }
blockdev::block_device *blockdev::search_device(const char *driver_name , max_t device_id) { return blockdev::search_device(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name) , device_id); }

bool blockdev::discard_device(blockdev::block_device *device) { return device->device_driver->device_container->discard_object(device); }

/// @brief Create empty device with essential informations
/// @param driver driver for device
/// @param storage_type type of storage
/// @return new empty device
blockdev::block_device *blockdev::create_empty_device(blockdev::block_device_driver *driver) {
    block_device *device = (block_device *)memory::pmem_alloc(sizeof(block_device));
    device->device_driver = driver;
    return device;
}

template <typename T> inline T *alloc_by_cnt(int cnt) { return (T *)memory::pmem_alloc(cnt*sizeof(T)); } 

void blockdev::designate_resources_count(blockdev::block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count) {
    if(io_port_count > 0) {
        device->resource.io_port_count = io_port_count;
        device->resource.io_ports = alloc_by_cnt<io_port>(io_port_count);
    }
    if(flags_count > 0) {
        device->resource.flags_count = flags_count;
        device->resource.flags = alloc_by_cnt<resource_flag_t>(flags_count);
    }
    if(etc_res_count > 0) {
        device->resource.etc_resources_count = etc_res_count;
        device->resource.etc_resources = alloc_by_cnt<etc_resource_t>(etc_res_count);
    }
}