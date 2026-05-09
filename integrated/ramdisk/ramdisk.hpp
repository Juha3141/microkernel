#ifndef _RAMDISK_DRIVER_HPP_
#define _RAMDISK_DRIVER_HPP_

#include <kernel/essentials.hpp>
#include <kernel/driver/block_device_driver.hpp>

struct ramdisk_driver : public block_device_driver {
    static struct block_device *create(max_t total_sector_count , max_t bytes_per_sectors , max_t physical_addr = 0x00);
    bool prepare(void) override;
    
    bool open(block_device *device) override;
    bool close(block_device *device) override;
    max_t read(block_device *device , max_t sector_address , max_t count , void *buffer) override;
    max_t write(block_device *device , max_t sector_address , max_t count , void *buffer) override;
    bool get_geometry(block_device *device , device_geometry &geometry) override;

    bool io_read(general_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(general_device *device , max_t command , max_t arguments) override;
};

struct ramdisk_info_s {
    max_t total_sector_count;
    max_t bytes_per_sector;
    
    max_t physical_address;
};

#endif