#ifndef _STORAGE_SYSTEM_HPP_
#define _STORAGE_SYSTEM_HPP_

#include <interface_type.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <interrupt.hpp>
#include <drivers/block_device_driver.hpp>

#include <debug.hpp>

// Hippopotomonstrosesquippedaliophobia
namespace storage_system {
    bool detect_partitions(blockdev::block_device *device); // detect partition
    void add_logical_device(blockdev::block_device_driver *driver , blockdev::block_device *device , const blockdev::partition_info_t partition_info);
    bool mount(blockdev::block_device *device); // detect file system
    bool unmount(blockdev::block_device *device);
};

#endif 