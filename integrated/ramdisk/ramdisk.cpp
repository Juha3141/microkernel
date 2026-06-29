#include "ramdisk.hpp"
#include <kernel/mem/kmem_manager.hpp>

#define RAMDISK_DRIVER_NAME "ramdisk"

/// @brief Creates a new ramdisk device and register it into the ramdisk driver
/// @param total_sector_count
/// @param bytes_per_sectors
/// @param physical_addr Physical(not linear) address to the RAMDisk image
/// @return Pointer to the created(and registered) block device
struct block_device *ramdisk_driver::create(max_t total_sector_count , max_t bytes_per_sectors , max_t physical_addr) {
    // Write some basic informations
    block_device *new_device = create_empty_device<block_device>();
    designate_resources_count<block_device>(new_device , 0 , 0 , 0 , 1);
    ramdisk_info_s *disk_info = (ramdisk_info_s *)memory::pmem_alloc(sizeof(ramdisk_info_s));

    // Write resource informations
    if(physical_addr == 0) physical_addr = (max_t)memory::pmem_alloc(total_sector_count*bytes_per_sectors);
    disk_info->total_sector_count = total_sector_count;
    disk_info->bytes_per_sector = bytes_per_sectors;
    disk_info->physical_address = physical_addr;
    // Resource 0 : ramdisk_info_s *
    new_device->resources.etc_resources[0] = (etc_resource_t)disk_info;

    if(dev::register_block_device(RAMDISK_DRIVER_NAME , new_device) == INVALID) return nullptr;
    return new_device;
}

bool ramdisk_driver::prepare(void) {
    return true; // There's actually nothing we have to do!
}

bool ramdisk_driver::open(block_device *device) {
    return true;
}

bool ramdisk_driver::close(block_device *device) {
    return true;
}

max_t ramdisk_driver::read(block_device *device , max_t sector_address , max_t count , void *buffer) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    max_t offset = 0 , mem_addr , tmp;
    max_t linear_addr = TO_VMEM(info->physical_address);
    max_t start_addr = linear_addr+(sector_address*info->bytes_per_sector);
    max_t ramdisk_limit = info->total_sector_count*info->bytes_per_sector+linear_addr;
    max_t sz   = min(start_addr+count*info->bytes_per_sector , ramdisk_limit)-start_addr;
    // check for address
    if(start_addr >= ramdisk_limit) return 0;
    
    memcpy(buffer , (void *)start_addr , sz);
    return sz;
}

max_t ramdisk_driver::write(block_device *device , max_t sector_address , max_t count , void *buffer) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    max_t offset = 0 , mem_addr , tmp;
    max_t linear_addr = TO_VMEM(info->physical_address);
    max_t start_addr = linear_addr+(sector_address*info->bytes_per_sector);
    max_t ramdisk_limit = info->total_sector_count*info->bytes_per_sector+linear_addr;
    max_t sz   = min(start_addr+count*info->bytes_per_sector , ramdisk_limit)-start_addr;
    // check for address
    if(start_addr >= ramdisk_limit) return 0;
    
    memcpy((void *)start_addr , buffer , sz);
    return sz;
}

bool ramdisk_driver::get_geometry(block_device *device , device_geometry &geometry) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    geometry.is_chs = false;

    geometry.block_size = info->bytes_per_sector;
    geometry.lba_total_block_count = info->total_sector_count;
    return true;
}

bool ramdisk_driver::io_read(general_device *device , max_t command , max_t arguments , max_t &data_out) { return false; }

bool ramdisk_driver::io_write(general_device *device , max_t command , max_t arguments) { return false; }

static void init_ramdisk_driver(void) {
    dev::register_driver(new ramdisk_driver , RAMDISK_DRIVER_NAME , block);
}

REGISTER_DRIVER(init_ramdisk_driver)