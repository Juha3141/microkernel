#ifndef _BLOCK_DEVICE_DRIVER_HPP_
#define _BLOCK_DEVICE_DRIVER_HPP_

#include <interface_type.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <interrupt.hpp>

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

namespace blockdev {
    // pre-define
    struct block_device_driver;
    class BlockDeviceContainer;

    struct block_device_driver_info {
        char driver_name[24];
        max_t block_size;
        max_t driver_id;
    };

    typedef struct partition_info_s {
        max_t physical_sector_start;
        max_t physical_sector_end;

        bool bootable;
    }partition_info_t;

    typedef enum storage_type_s {
        physical=0 , logical=1
    }storage_type_t;

    struct storage_info_t {
        max_t partition_driver_id;
        storage_type_t storage_type;
        
        /* For logical storage */
        max_t partition_id;
        // If physical_sector_start == 0 --> Physical disk, "physical_sector_end" describes the total sector count
        // If physical_sector_start != 0 --> Logical disk
        partition_info_t partition_info; 

        /* Common */
        blockdev::BlockDeviceContainer *logical_block_devs;
    };
    struct block_device {
        max_t id;
        block_device_driver *device_driver;
        device_resources resource;
        
        storage_info_t storage_info;

        max_t mount_id;
        // FileSystemDriver *fs_driver;
    };
    struct block_device_driver {
        virtual bool prepare(void) = 0;
        virtual max_t read(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual max_t write(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual bool io_read(block_device *device , max_t command , max_t argument) = 0;
        virtual bool io_write(block_device *device , max_t command , max_t argument) = 0;

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

    max_t register_driver(block_device_driver *driver , const char *driver_name);
    block_device_driver *search_driver(const char *driver_name);
    block_device_driver *search_driver(max_t driver_id);

    max_t discard_driver(const char *driver_name);
    max_t discard_driver(max_t driver_id);
    
    max_t register_device(block_device_driver *driver , block_device *device);
    max_t register_device(const char *driver_name , block_device *device);
    max_t register_device(max_t driver_id , block_device *device);
    block_device *search_device(block_device_driver *driver , max_t device_id);
    block_device *search_device(const char *driver_name , max_t device_id);
    block_device *search_device(max_t driver_id , max_t device_id);

    bool discard_device(block_device *device);

    block_device *create_empty_device(block_device_driver *driver);
    void designate_resources_count(block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count);
};

#endif