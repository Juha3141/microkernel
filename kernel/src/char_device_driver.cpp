#include <char_device_driver.hpp>
#include <linked_list.hpp>
#include <kmem_manager.hpp>

#include <debug.hpp>

void chardev::init(void) {
    // CharDeviceDriverContainer : global container for kernel
    GLOBAL_OBJECT(CharDeviceDriverContainer)->init(512);
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
max_t chardev::register_driver(chardev::char_device_driver *driver , const char *driver_name) {
    CharDeviceDriverContainer *driver_container = GLOBAL_OBJECT(CharDeviceDriverContainer);
    max_t id = driver_container->register_object(driver); // register driver to global container
    driver->driver_id = id;
    if(id == INVALID) return INVALID; 
    driver->device_container = new CharDeviceContainer;
    // assign new local device container
    // Driver contains its devices
    strcpy(driver->driver_name , driver_name);
    
    debug::out::printf_function(DEBUG_TEXT , "bdev::reg_drv" , "Registered character device driver, id : %d name : \"%s\"\n" , driver->driver_id , driver->driver_name);
    return driver->driver_id;
}

///@brief some bridge-like functions(just basic stuff)

chardev::char_device_driver *chardev::search_driver(const char *driver_name) { 
    max_t id = GLOBAL_OBJECT(CharDeviceDriverContainer)->search<const char *>([](char_device_driver *driver , const char *name) { return (bool)(strcmp(driver->driver_name , name) == 0); } , driver_name);  
    return GLOBAL_OBJECT(CharDeviceDriverContainer)->get_object(id);
}

chardev::char_device_driver *chardev::search_driver(max_t driver_id) { return GLOBAL_OBJECT(CharDeviceDriverContainer)->get_object(driver_id); }

max_t chardev::discard_driver(const char *driver_name) { return GLOBAL_OBJECT(CharDeviceDriverContainer)->discard_object(search_driver(driver_name)); }
max_t chardev::discard_driver(max_t driver_id) { return GLOBAL_OBJECT(CharDeviceDriverContainer)->discard_object(GLOBAL_OBJECT(CharDeviceDriverContainer)->get_object(driver_id)); }

/// @brief Registeres device to driver (kernel)
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t chardev::register_device(chardev::char_device_driver *driver , chardev::char_device *device) {
    CharDeviceDriverContainer *driver_container = CharDeviceDriverContainer::get_self();

    device->id = driver->device_container->register_object(device);
    if(device->id == INVALID) { debug::out::printf(DEBUG_ERROR , "invalid id!\n"); return INVALID; }
    
    device->device_driver = driver;
    return device->id;
}

/// @brief Other form of register_device

max_t chardev::register_device(const char *driver_name , chardev::char_device *device) { return chardev::register_device(search_driver(driver_name) , device); }
max_t chardev::register_device(max_t driver_id , chardev::char_device *device) { return chardev::register_device(GLOBAL_OBJECT(CharDeviceDriverContainer)->get_object(driver_id) , device); }

chardev::char_device *chardev::search_device(max_t driver_id , max_t device_id) { return search_driver(driver_id)->device_container->get_object(device_id); }
chardev::char_device *chardev::search_device(chardev::char_device_driver *driver , max_t device_id) {  return driver->device_container->get_object(device_id); }
chardev::char_device *chardev::search_device(const char *driver_name , max_t device_id) { return chardev::search_device(search_driver(driver_name) , device_id); }

bool chardev::discard_device(chardev::char_device *device) { return device->device_driver->device_container->discard_object(device); }