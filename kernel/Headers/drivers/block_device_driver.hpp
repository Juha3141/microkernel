#ifndef _BLOCK_DEVICE_DRIVER_HPP_
#define _BLOCK_DEVICE_DRIVER_HPP_

#include <interface_type.hpp>
#include <drivers/storage_device_driver.hpp>

namespace blockdev {
    // pre-define
    struct block_device_driver;
    class BlockDeviceContainer;

    struct block_device_driver_info {
        char driver_name[24];
        max_t block_size;
        max_t driver_id;
    };

    struct block_device {
        max_t id;
        block_device_driver *device_driver;
        device_resources resource;
    };
    struct block_device_driver {
        virtual bool prepare(void) = 0;
        virtual max_t read(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual max_t write(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual bool io_read(block_device *device , max_t command , ...) = 0;
        virtual bool io_write(block_device *device , max_t command , ...) = 0;

        class BlockDeviceContainer *device_container;
        block_device_driver_info driver_info;
    };
    
    class BlockDeviceContainer : public ObjectManager<block_device> {};
    class BlockDeviceDriverContainer : public ObjectManager<block_device_driver> {
        public:
            SINGLETON_PATTERN_PMEM(BlockDeviceDriverContainer);

            max_t register_driver(block_device_driver *driver) {
                driver->driver_info.driver_id = ObjectManager<block_device_driver>::register_object(driver);
                return driver->driver_info.driver_id;
            }
            block_device_driver *search_by_name(const char *driver_name) {
                max_t id = search<const char *>([](block_device_driver *driver , const char *name) { return (bool)(strcmp(driver->driver_info.driver_name , name) == 0); } , driver_name); 
                return get_object(id);
            }
    };
    void init(void);
};

#endif