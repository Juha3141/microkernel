#ifndef _BLOCK_DEVICE_DRIVER_HPP_
#define _BLOCK_DEVICE_DRIVER_HPP_

#include <kernel/driver/device_driver.hpp>
#include <kernel/driver/block_device_scheduler.hpp>

// pre
struct block_device_driver;
struct block_device;
namespace fsdev { struct file_system_driver; }

typedef struct partition_info_s {
    max_t physical_sector_start;
    max_t physical_sector_end;
    bool bootable;
}partition_info_t;

typedef enum storage_type_s {
    storage_physical=0 , storage_logical=1
}storage_type_t;

struct device_geometry {
    max_t lba_total_block_count;
    max_t block_size;
    bool is_chs;
    max_t cylinder;
    max_t head;
    max_t sector;
};

struct storage_info_t {
    max_t partition_driver_id;
    storage_type_t storage_type; // physical/logical
    
    /* For logical storage */
    max_t partition_id;
    // If physical_sector_start == 0 --> Physical disk, "physical_sector_end" describes the total sector count
    // If physical_sector_start != 0 --> Logical disk
    partition_info_t partition_info; 

    /* Common */
    fsdev::file_system_driver *fs_driver;
    FixedArray<block_device*> *logical_block_devs;
};

struct block_device : general_device {
    // Geometry information
    device_geometry geometry;
    // Storage information
    storage_info_t storage_info;
    block_device_driver *driver;
    max_t mount_id;
};

struct block_device_driver : public device_driver {
    virtual bool open(block_device *device) = 0;
    virtual bool close(block_device *device) = 0;
    virtual max_t read(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
    virtual max_t write(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
    virtual bool get_geometry(block_device *device , device_geometry &geometry) = 0;
};

namespace dev {
    max_t register_block_device(block_device_driver *driver , block_device *device);
    max_t register_block_device(const char *driver_name , block_device *device);
    max_t register_block_device(max_t driver_id , block_device *device);
}

#endif