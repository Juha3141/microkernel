#include <drivers/storage_system.hpp>
#include <drivers/block_device_driver.hpp>
#include <drivers/partition_driver.hpp>

bool storage_system::detect_partitions(blockdev::block_device *device) {
    max_t partitiondrv_id = identify_partition_driver(device);
    PartitionDriver *partition_driver = get_partition_identifier(device);
    int partition_count = 0;
    DataLinkedList<blockdev::partition_info_t>partition_info_list;
    if(partition_driver != 0x00) {
        partition_count = partition_driver->get_partitions_list(device , partition_info_list);
        device->storage_info.partition_driver_id = partitiondrv_id;
        device->storage_info.logical_block_devs = new blockdev::BlockDeviceContainer;
        device->storage_info.logical_block_devs->init(72); // maximum logical storages count

        DataLinkedList<blockdev::partition_info_t>::node_s *ptr = partition_info_list.get_start_node();
        while(ptr != 0x00) {
            add_logical_device(device->device_driver , device , ptr->data);
            ptr = ptr->next;
        }
        return true; // partition
    }
    return false; // no partition
}

void storage_system::add_logical_device(blockdev::block_device_driver *driver , blockdev::block_device *device , const blockdev::partition_info_t partition_info) {
    blockdev::block_device *new_logical_device;
    new_logical_device = blockdev::create_empty_device(driver);
    new_logical_device->id = device->id;
    memcpy(&new_logical_device->resource , &device->resource , sizeof(device_resources));
    memcpy(&new_logical_device->storage_info.partition_info , &partition_info , sizeof(blockdev::partition_info_t));
    
    new_logical_device->storage_info.partition_id = device->storage_info.logical_block_devs->register_object(new_logical_device);
    blockdev::register_device(driver , new_logical_device);
}