#include <kernel/driver/block_device_driver.hpp>
#include <linked_list.hpp>
#include <kernel/vfs/storage_system.hpp>

#include <kernel/mem/kmem_manager.hpp>

#include <kernel/debug.hpp>

FixedArray<blockdev::block_device_driver*> *bdevdrv_container;

void blockdev::init(void) {
    // BlockDeviceDriverContainer : global container for kernel
    bdevdrv_container = memory::new_global_object<FixedArray<blockdev::block_device_driver*>>();
    bdevdrv_container->init(512);
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
max_t blockdev::register_driver(blockdev::block_device_driver *driver , const char *driver_name) {
    max_t id = bdevdrv_container->add(driver); // register driver to global container
    driver->driver_id = id;
    if(id == INVALID) { return INVALID; }
    
    driver->device_container = new FixedArray<block_device*>();
    driver->device_container->init(256);
    // assign new local device container
    // Driver contains its devices
    strcpy(driver->driver_name , driver_name);
    driver->prepare();
    
    debug::out::printf("Registered device driver, id : %d name : \"%s\"\n" , driver->driver_id , driver->driver_name);
    return driver->driver_id;
}

///@brief some bridge-like functions(just basic stuff)

blockdev::block_device_driver *blockdev::search_driver(const char *driver_name) { 
    max_t id = bdevdrv_container->search(
        [driver_name](block_device_driver *&driver) { return (bool)(strcmp(driver->driver_name , driver_name) == 0); }
    );  
    return bdevdrv_container->get(id);
}

blockdev::block_device_driver *blockdev::search_driver(max_t driver_id) { return bdevdrv_container->get(driver_id); }

max_t blockdev::discard_driver(const char *driver_name) { return bdevdrv_container->discard(search_driver(driver_name)); }
max_t blockdev::discard_driver(max_t driver_id) { return bdevdrv_container->discard(bdevdrv_container->get(driver_id)); }

/// @brief Registeres device to driver (kernel)
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t blockdev::register_device(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    if(driver->get_geometry(device , device->geometry) == false) return INVALID;
    device->id = driver->device_container->add(device);
    if(device->id == INVALID) { debug::out::printf(DEBUG_ERROR , "invalid id!\n"); return INVALID; }
    
    device->device_driver = driver;
    storage_system::detect_partitions(device);
    return device->id;
}

/// @brief Other form of register_device

max_t blockdev::register_device(const char *driver_name , blockdev::block_device *device) { return blockdev::register_device(search_driver(driver_name) , device); }
max_t blockdev::register_device(max_t driver_id , blockdev::block_device *device) {
    blockdev::block_device_driver *driver = bdevdrv_container->get(driver_id);
    if(driver == nullptr) {
        debug::out::printf(DEBUG_WARNING , "Unable to find block device driver with driver_id=%d\n" , driver_id);
        return INVALID;
    }
    return blockdev::register_device(driver , device);
}

blockdev::block_device *blockdev::search_device(max_t driver_id , max_t device_id) { return search_driver(driver_id)->device_container->get(driver_id); }
blockdev::block_device *blockdev::search_device(blockdev::block_device_driver *driver , max_t device_id) { return driver->device_container->get(device_id); }
blockdev::block_device *blockdev::search_device(const char *driver_name , max_t device_id) { return blockdev::search_device(search_driver(driver_name) , device_id); }

bool blockdev::discard_device(blockdev::block_device *device) { return device->device_driver->device_container->discard(device);  }