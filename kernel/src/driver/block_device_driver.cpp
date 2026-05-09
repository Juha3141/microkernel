#include <kernel/driver/block_device_driver.hpp>
#include <linked_list.hpp>
#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>

#include <kernel/mem/kmem_manager.hpp>

#include <kernel/debug.hpp>

/// @brief Registeres device to driver (kernel)
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t dev::register_block_device(block_device_driver *driver , block_device *device) {
    if(driver->get_geometry(device , device->geometry) == false) return INVALID;
    device->driver = driver;
    
    // Detect partitions
    device->id = driver->device_container->add(device);
    if(device->id == INVALID) { debug::out::printf(DEBUG_ERROR , "invalid id!\n"); return INVALID; }
    storage_system::detect_partitions(device);

    // Detect file system
    device->storage_info.fs_driver = fsdev::detect_fs(device);
    return device->id;
}

max_t dev::register_block_device(const char *driver_name , block_device *device) { return dev::register_block_device((block_device_driver *)dev::search_driver(driver_name) , device); }
max_t dev::register_block_device(max_t driver_id , block_device *device) { return dev::register_block_device((block_device_driver *)dev::search_driver(driver_id) , device); }