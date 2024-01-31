#ifndef _PARTITION_DRIVER_HPP_
#define _PARTITION_DRIVER_HPP_

#include <interface_type.hpp>
#include <object_manager.hpp>
#include <linked_list.hpp>
#include <drivers/storage_device_driver.hpp>

namespace storagedev {
    struct PartitionDriver {
        virtual bool identify(storagedev::storage_device *device) = 0;
	    virtual int get_partitions_count(storagedev::storage_device *device) = 0;
    	virtual int get_partitions_list(storagedev::storage_device *device , DataLinkedList<storagedev::partition_info> &partition_info_list) = 0;
    	virtual bool create_partition(storagedev::storage_device *device , storagedev::partition_info partition) = 0;
        virtual bool remove_partition(storagedev::storage_device *device , storagedev::partition_info partition) = 0;
        virtual bool modify_partition(storagedev::storage_device *device , storagedev::partition_info old_partition , storagedev::partition_info new_partition_info) = 0;

        const char *driver_name;
    };

    struct PartitionDriverContainer : ObjectManager<PartitionDriver> {
        SINGLETON_PATTERN_PMEM(PartitionDriverContainer);

        friend max_t identify_partition_driver(storagedev::storage_device *device);
    };

    max_t register_partition_driver(storagedev::PartitionDriver *partition_driver);
    max_t identify_partition_driver(storagedev::storage_device *device);
    PartitionDriver *get_partition_identifier(storagedev::storage_device *device);
}

#endif