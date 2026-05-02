#include <kernel/driver/device_driver.hpp>

extern qword __drivers_init_start__;
extern qword __drivers_init_end__;

extern qword __fs_drivers_init_start__;
extern qword __fs_drivers_init_end__;

void dev::register_file_system_drivers(void) { CALL_FPTR_FROM_SECTION(__fs_drivers_init_start__ , __fs_drivers_init_end__); }
void dev::register_kernel_drivers(void)      { CALL_FPTR_FROM_SECTION(__drivers_init_start__ , __drivers_init_end__); }

FixedArray<device_driver *>* device_driver_container;

void dev::init(void) {
    // BlockDeviceDriverContainer : global container for kernel
    device_driver_container = memory::new_global_object<FixedArray<device_driver*>>();
    device_driver_container->init(512);
}

///@brief some bridge-like functions(just basic stuff)
device_driver *dev::search_driver(const char *driver_name) { 
    max_t id = device_driver_container->search(
        [driver_name](device_driver *&driver) { return (bool)(strcmp(driver->driver_name , driver_name) == 0); }
    );  
    return device_driver_container->get(id);
}

device_driver *dev::search_driver(max_t driver_id) { return device_driver_container->get(driver_id); }

max_t dev::discard_driver(const char *driver_name) { return device_driver_container->discard(search_driver(driver_name)); }
max_t dev::discard_driver(max_t driver_id) { return device_driver_container->discard(device_driver_container->get(driver_id)); }

/// @brief Other forms of register_device
max_t dev::register_device(const char *driver_name , general_device *device) { return dev::register_device(search_driver(driver_name) , device); }
max_t dev::register_device(max_t driver_id , general_device *device) {
    device_driver *driver = device_driver_container->get(driver_id);
    if(driver == nullptr) {
        debug::out::printf(DEBUG_WARNING , "Unable to find block device driver with driver_id=%d\n" , driver_id);
        return INVALID;
    }
    return dev::register_device(driver , device);
}

general_device *dev::search_device(max_t driver_id , max_t device_id) { return search_driver(driver_id)->device_container->get(driver_id); }
general_device *dev::search_device(const char *driver_name , max_t device_id) { return dev::search_device(search_driver(driver_name) , device_id); }

bool dev::discard_device(general_device *device) { return device->driver->device_container->discard(device);  }