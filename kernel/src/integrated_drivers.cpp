/*
#include <integrated/integrated_drivers.hpp>

#include <integrated/mbr.hpp>
#include <integrated/gpt.hpp>

#include <integrated/ramdisk.hpp>
#include <integrated/ide.hpp>

void integrated::register_drivers(void) {
    storage_system::register_partition_driver(new MBRPartitionDriver);
    storage_system::register_partition_driver(new GPTPartitionDriver);
    
    // register block device drivers
    ramdisk_driver::init_driver();
    ide_driver::register_driver();
    ide_cd_driver::register_driver();

    // register character device drivers
}
*/