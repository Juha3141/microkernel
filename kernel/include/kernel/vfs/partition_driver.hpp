/**
 * @file partition_driver.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _PARTITION_DRIVER_HPP_
#define _PARTITION_DRIVER_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/vfs/storage_system.hpp>

#include <object_manager.hpp>
#include <linked_list.hpp>

namespace storage_system {
    struct PartitionDriver {
        virtual bool identify(blockdev::block_device *device) = 0;
	    virtual int get_partitions_count(blockdev::block_device *device) = 0;
    	virtual int get_partitions_list(blockdev::block_device *device , DataLinkedList<blockdev::partition_info_t> &partition_info_list) = 0;
    	virtual bool create_partition(blockdev::block_device *device , blockdev::partition_info_t partition) = 0;
        virtual bool remove_partition(blockdev::block_device *device , blockdev::partition_info_t partition) = 0;
        virtual bool modify_partition(blockdev::block_device *device , blockdev::partition_info_t old_partition , blockdev::partition_info_t new_partition_info) = 0;

        const char *driver_name;
    };

    struct PartitionDriverContainer : ObjectManager<PartitionDriver> {
        SINGLETON_PATTERN_PMEM(PartitionDriverContainer);

        friend max_t identify_partition_driver(blockdev::block_device *device);
    };

    max_t register_partition_driver(PartitionDriver *partition_driver);
    max_t identify_partition_driver(blockdev::block_device *device);
    PartitionDriver *get_partition_identifier(blockdev::block_device *device);
    PartitionDriver *get_partition_identifier(max_t identifier_id);
}

#endif