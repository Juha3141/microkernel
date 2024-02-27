#ifndef _RAMDISK_DRIVER_HPP_
#define _RAMDISK_DRIVER_HPP_

#include <kernel/interface_type.hpp>
#include <block_device_driver.hpp>

struct ramdisk_driver : public blockdev::block_device_driver {
    static void init_driver(void);

    static struct blockdev::block_device *create(max_t total_sector_count , max_t bytes_per_sectors , max_t physical_addr = 0x00);
    bool prepare(void) override;
    max_t read(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) override;
    max_t write(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) override;
    bool get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) override;

    bool io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(blockdev::block_device *device , max_t command , max_t arguments) override;
};

struct ramdisk_info_s {
    max_t total_sector_count;
    max_t bytes_per_sector;
    
    max_t physical_address;
};

#endif