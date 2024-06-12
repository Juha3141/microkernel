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
    blockdev::block_device *get_physical_super_device(blockdev::block_device *logical_device);

    struct logical_block_device_driver : public blockdev::block_device_driver {
        void set_super_driver(struct blockdev::block_device_driver *driver) {
            super_driver = driver;
            super_driver->driver_id = driver->driver_id;
            strcpy(super_driver->driver_name , driver->driver_name);
        }
        bool init(void) { return false; }
        bool prepare(void) override { debug::out::printf(DEBUG_WARNING , "logical_storage_device_driver::prepare : not allowed\n"); return false; }
        max_t read(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) override {
            struct blockdev::block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            
            sector_address += device->storage_info.partition_info.physical_sector_start;
            
            // Get physical storage pointer of logical storage
            return physical_super_device->device_driver->read(physical_super_device , sector_address , count , buffer);
        }
        max_t write(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) override {
            blockdev::block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            
            sector_address += device->storage_info.partition_info.physical_sector_start;
            
            return physical_super_device->device_driver->write(physical_super_device , sector_address , count , buffer);
        }
        bool get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) override {
            blockdev::block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            return physical_super_device->device_driver->get_geometry(device , geometry);
        }
        bool io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) override {
            blockdev::block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            return physical_super_device->device_driver->io_read(device , command , argument , data_out);
        }
        bool io_write(blockdev::block_device *device , max_t command , max_t argument) override {
            blockdev::block_device *physical_super_device = get_physical_super_device(device);
            if(!check(physical_super_device , device)) return 0x00;
            return physical_super_device->device_driver->io_write(device , command , argument);
        }
        blockdev::block_device_driver *super_driver;

        private:
            inline bool check(blockdev::block_device *physical_super_device , blockdev::block_device *device) {
                if((physical_super_device == 0x00)
                 ||(device->storage_info.storage_type == blockdev::physical)
                 ||(device->device_driver->driver_id == INVALID)) return false;
                return true;
            }
    };
    void init(void);

    bool detect_partitions(blockdev::block_device *device); // detect partition
    void add_logical_device(blockdev::block_device_driver *driver , blockdev::block_device *device , const blockdev::partition_info_t partition_info);
    bool mount(blockdev::block_device *device); // detect file system
    bool unmount(blockdev::block_device *device);
};

#endif 