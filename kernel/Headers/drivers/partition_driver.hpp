#ifndef _PARTITION_DRIVER_HPP_
#define _PARTITION_DRIVER_HPP_

#include <interface_type.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <drivers/block_device_driver.hpp>
#include <drivers/storage_system.hpp>

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