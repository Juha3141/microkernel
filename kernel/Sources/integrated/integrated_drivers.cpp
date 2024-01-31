#include <integrated/integrated_drivers.hpp>

#include <integrated/mbr.hpp>
#include <integrated/gpt.hpp>

#include <integrated/ramdisk.hpp>

void integrated::register_drivers(void) {
    storagedev::register_partition_driver(new MBRPartitionDriver);
    storagedev::register_partition_driver(new GPTPartitionDriver);
    
    // register ramdisk driver
    ramdisk_driver::init_driver();
}