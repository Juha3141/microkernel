#ifndef _BLOCK_DEVICE_DRIVER_HPP_
#define _BLOCK_DEVICE_DRIVER_HPP_

#include <interface_type.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <interrupt.hpp>

#include <debug.hpp>

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
    struct device_geometry {
        max_t total_sector_count;
        max_t bytes_per_sector;
        
        char device_name[32];
        char manufacturer_name[32];

        // To-do : add more
    };
    struct partition_info {
        max_t physical_sector_start;
        max_t physical_sector_end;
        bool bootable;
    };
    typedef enum storage_type_s {
        physical=0 , logical=1
    }storage_type_t;

    // pre-define
    struct block_device_driver;
    class BlockDeviceContainer;

    struct block_device {
        /* For physical device */
        max_t id;
        block_device_driver *device_driver;
        // FileSystemDriver fs_driver;
        
        bool mounted;
        max_t mount_id;
        max_t partition_driver_id;
        storage_type_t storage_type;
        
        /* For logical storage */
        max_t partition_id;
        partition_info logical_partition_info;

        /* Common */
        device_resources resource;
        device_geometry geomtery;
        BlockDeviceContainer *logical_devices;
    };

    struct block_device_driver {
        friend class BlockDeviceDriverContainer;

        virtual bool prepare(void) = 0; 
        virtual max_t read_sector(block_device *device , max_t sector_address , max_t count , void *buffer) = 0;
        virtual max_t write_sector(block_device *device , max_t sector_address , max_t count , void *buffer) = 0;
        virtual bool get_device_geometry(block_device *device , device_geometry *geometry) = 0;
        
        class BlockDeviceContainer *device_container;

        max_t driver_id;
        char driver_name[24]; 
    };
    
    block_device *get_physical_super_device(block_device *device);
    
    struct logical_block_device_driver : public block_device_driver {
        void set_super_driver(struct block_device_driver *driver) {
            super_driver = driver;
            driver_id = driver->driver_id;
        }
        bool init(void) { return false; }
        bool prepare(void) { debug::out::printf(DEBUG_WARNING , "logical_block_device_driver::prepare : not allowed\n"); return false; }
        max_t read_sector(block_device *device , max_t sector_address , max_t count , void *buffer) override {
            struct block_device *physical_super_device = get_physical_super_device(device);
            if((physical_super_device == 0x00)
             ||(device->storage_type == physical)
             ||(device->device_driver->driver_id == INVALID)) return 0x00;
            
            sector_address += device->logical_partition_info.physical_sector_start;
            
            // Get physical storage pointer of logical storage
            return physical_super_device->device_driver->read_sector(physical_super_device , sector_address , count , buffer);
        }
        max_t write_sector(block_device *device , max_t sector_address , max_t count , void *buffer) override {
            struct block_device *physical_super_device = get_physical_super_device(device);
            if((physical_super_device == 0x00)
             ||(device->storage_type == physical)
             ||(device->device_driver->driver_id == INVALID)) return 0x00;
            
            sector_address += device->logical_partition_info.physical_sector_start;
            
            return physical_super_device->device_driver->write_sector(physical_super_device , sector_address , count , buffer);
        }
        bool get_device_geometry(block_device *device , device_geometry *geometry) override {
            if(super_driver == 0x00) {
                return false;
            }
            return super_driver->get_device_geometry(device , geometry);
        }
        
        struct block_device_driver *super_driver;
    };

    class BlockDeviceContainer : public ObjectManager<block_device> {};
    class BlockDeviceDriverContainer : public ObjectManager<block_device_driver> {
        public:
            SINGLETON_PATTERN_PMEM(BlockDeviceDriverContainer);

            max_t register_driver(block_device_driver *driver) {
                driver->driver_id = ObjectManager<block_device_driver>::register_object(driver);
                return driver->driver_id;
            }
            block_device_driver *search_by_name(const char *driver_name) {
                max_t id = search<const char *>([](block_device_driver *driver , const char *name) { return (bool)(strcmp(driver->driver_name , name) == 0); } , driver_name); 
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
    block_device *search_device(block_device_driver *driver , max_t device_id , max_t partition_id=INVALID);
    block_device *search_device(max_t driver_id , max_t device_id , max_t partition_id=INVALID);
    block_device *search_device(const char *driver_name , max_t device_id , max_t partition_id=INVALID);

    bool discard_device(block_device *device);

    block_device *create_empty_device(block_device_driver *driver , storage_type_t storage_type);
    void designate_resources_count(blockdev::block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count);
    void add_logical_device(block_device_driver *driver , block_device *device , DataLinkedList<blockdev::partition_info> &partitions);
    block_device *get_physical_super_device(block_device *device);
};

#endif 