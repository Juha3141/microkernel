#ifndef _STORAGE_SYSTEM_HPP_
#define _STORAGE_SYSTEM_HPP_

#include <kernel/essentials.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/driver/block_device_driver.hpp>

#include <object_manager.hpp>
#include <linked_list.hpp>

#include <kernel/debug.hpp>

// Hippopotomonstrosesquippedaliophobia
namespace storage_system {
    block_device *get_physical_super_device(block_device *logical_device);

    struct logical_block_device_driver : public block_device_driver {
        void set_super_driver(struct block_device_driver *driver) {
            super_driver = driver;
            super_driver->driver_id = driver->driver_id;
            strcpy(super_driver->driver_name , driver->driver_name);
        }
        bool init(void) { return false; }
        bool prepare(void) override { debug::out::printf(DEBUG_WARNING , "logical_storage_device_driver::prepare : not allowed\n"); return false; }
        
        bool open(block_device *device) override {
            struct block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return false;
            // Get physical storage pointer of logical storage
            return physical_super_device->driver->open(physical_super_device);
        }
        bool close(block_device *device) override {
            struct block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return false;
            // Get physical storage pointer of logical storage
            return physical_super_device->driver->close(physical_super_device);
        }
        max_t read(block_device *device , max_t sector_address , max_t count , void *buffer) override {
            struct block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            
            sector_address += device->storage_info.partition_info.physical_sector_start;
            
            // Get physical storage pointer of logical storage
            return physical_super_device->driver->read(physical_super_device , sector_address , count , buffer);
        }
        max_t write(block_device *device , max_t sector_address , max_t count , void *buffer) override {
            block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            
            sector_address += device->storage_info.partition_info.physical_sector_start;
            
            return physical_super_device->driver->write(physical_super_device , sector_address , count , buffer);
        }
        bool get_geometry(block_device *device , device_geometry &geometry) override {
            block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            return physical_super_device->driver->get_geometry(device , geometry);
        }
        bool io_read(general_device *device , max_t command , max_t argument , max_t &data_out) override {
            block_device *physical_super_device = get_physical_super_device((block_device *)device);
            if(!check(physical_super_device , (block_device *)device)) return 0x00;
            return physical_super_device->driver->io_read(device , command , argument , data_out);
        }
        bool io_write(general_device *device , max_t command , max_t argument) override {
            block_device *physical_super_device = get_physical_super_device((block_device *)device);
            if(!check(physical_super_device , (block_device *)device)) return 0x00;
            return physical_super_device->driver->io_write(device , command , argument);
        }
        block_device_driver *super_driver;

        private:
            inline bool check(block_device *physical_super_device , block_device *device) {
                if((physical_super_device == 0x00)
                 ||(device->storage_info.storage_type == storage_physical)
                 ||(device->driver->driver_id == INVALID)) return false;
                return true;
            }
    };
    void init(void);

    bool detect_partitions(block_device *device); // detect partition
    void add_logical_device(block_device_driver *driver , block_device *device , const partition_info_t partition_info);
    bool mount(block_device *device); // detect file system
    bool unmount(block_device *device);
};

#endif 