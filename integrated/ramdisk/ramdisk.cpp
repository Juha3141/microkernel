#include "ramdisk.hpp"
#include <kernel/kmem_manager.hpp>

static max_t ramdisk_driver_id = 0x00;

void ramdisk_driver::register_driver(void) {
    ramdisk_driver_id = blockdev::register_driver(new ramdisk_driver , "rd");
}

struct blockdev::block_device *ramdisk_driver::create(max_t total_sector_count , max_t bytes_per_sectors , max_t physical_addr) {
    // Write some basic informations
    blockdev::block_device *new_device = blockdev::create_empty_device();
    blockdev::designate_resources_count(new_device , 0 , 0 , 0 , 1);
    new_device->device_driver = blockdev::search_driver(ramdisk_driver_id);
    ramdisk_info_s *disk_info = (ramdisk_info_s *)memory::pmem_alloc(sizeof(ramdisk_info_s));

    // Write resource informations
    if(physical_addr == 0) physical_addr = (max_t)memory::pmem_alloc(total_sector_count*bytes_per_sectors);
    disk_info->total_sector_count = total_sector_count;
    disk_info->bytes_per_sector = bytes_per_sectors;
    disk_info->physical_address = physical_addr;
    // Resource 0 : ramdisk_info_s *
    new_device->resources.etc_resources[0] = (etc_resource_t)disk_info;

    return new_device;
}

bool ramdisk_driver::prepare(void) {
    return true; // There's actually nothing we have to do!
}

max_t ramdisk_driver::read(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    max_t offset = 0 , mem_addr , tmp;
    max_t start_addr = info->physical_address+(sector_address*info->bytes_per_sector);
    for(mem_addr = start_addr; mem_addr < (start_addr+(count*info->bytes_per_sector)); mem_addr += sizeof(max_t)) {
        if(mem_addr >= (info->physical_address+(info->total_sector_count*info->bytes_per_sector))) break;
        *((max_t *)((max_t)buffer+offset)) = *((max_t *)mem_addr);
        offset += sizeof(max_t);
    }
    return (mem_addr-start_addr);
}

max_t ramdisk_driver::write(blockdev::block_device *device , max_t sector_address , max_t count , void *buffer) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    max_t offset = 0 , mem_addr , tmp;
    max_t start_addr = info->physical_address+(sector_address*info->bytes_per_sector);
    for(mem_addr = start_addr; mem_addr < (start_addr+(count*info->bytes_per_sector)); mem_addr += sizeof(max_t)) {
        if(mem_addr >= (info->physical_address+(info->total_sector_count*info->bytes_per_sector))) break;
        *((max_t *)mem_addr) = *((max_t *)((max_t)buffer+offset));
        offset += sizeof(max_t);
    }
    return (mem_addr-start_addr);
}

bool ramdisk_driver::get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) {
    ramdisk_info_s *info = (ramdisk_info_s *)device->resources.etc_resources[0];
    geometry.is_chs = false;

    geometry.block_size = info->bytes_per_sector;
    geometry.lba_total_block_count = info->total_sector_count;
    return true;
}

bool ramdisk_driver::io_read(blockdev::block_device *device , max_t command , max_t arguments , max_t &data_out) { return false; }

bool ramdisk_driver::io_write(blockdev::block_device *device , max_t command , max_t arguments) { return false; }