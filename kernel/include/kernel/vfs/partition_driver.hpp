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

#include <kernel/essentials.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/vfs/storage_system.hpp>

#include <object_manager.hpp>
#include <linked_list.hpp>

namespace storage_system {
    struct PartitionDriver {
        virtual bool identify(block_device *device) = 0;
	    virtual int get_partitions_count(block_device *device) = 0;
    	virtual int get_partitions_list(block_device *device , LinkedList<partition_info_t> &partition_info_list) = 0;
    	virtual bool create_partition(block_device *device , partition_info_t partition) = 0;
        virtual bool remove_partition(block_device *device , partition_info_t partition) = 0;
        virtual bool modify_partition(block_device *device , partition_info_t old_partition , partition_info_t new_partition_info) = 0;

        char driver_name[16];
    };

    struct PartitionDriverContainer : FixedArray<PartitionDriver*> {
        friend max_t identify_partition_driver(block_device *device);
    };

    max_t register_partition_driver(PartitionDriver *partition_driver , const char *driver_name);
    max_t identify_partition_driver(block_device *device);
    PartitionDriver *get_partition_identifier(block_device *device);
    PartitionDriver *get_partition_identifier(max_t identifier_id);
}

#endif