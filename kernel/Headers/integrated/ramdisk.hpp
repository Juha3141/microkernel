#ifndef _RAMDISK_DRIVER_HPP_
#define _RAMDISK_DRIVER_HPP_

#include <interface_type.hpp>
#include <drivers/storage_device_driver.hpp>

struct ramdisk_driver : public storagedev::storage_device_driver {
    static void init_driver(void);

    static struct storagedev::storage_device *create(max_t total_sector_count , max_t bytes_per_sectors , max_t physical_addr = 0x00);
    bool prepare(void) override;
    max_t read_sector(storagedev::storage_device *device , max_t sector_address , max_t count , void *buffer) override;
    max_t write_sector(storagedev::storage_device *device , max_t sector_address , max_t count , void *buffer) override;
    bool get_device_geometry(storagedev::storage_device *device , storagedev::device_geometry *geometry) override; 
};

struct ramdisk_info_s {
    max_t total_sector_count;
    max_t bytes_per_sector;
    
    max_t physical_address;
};

#endif