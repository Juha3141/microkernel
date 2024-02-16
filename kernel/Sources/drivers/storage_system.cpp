#include <drivers/storage_system.hpp>
#include <drivers/block_device_driver.hpp>
#include <drivers/partition_driver.hpp>

void storage_system::init(void) {
    PartitionDriverContainer::get_self()->init(64);
}

bool storage_system::detect_partitions(blockdev::block_device *device) {
    max_t partitiondrv_id = identify_partition_driver(device);
    PartitionDriver *partition_driver = get_partition_identifier(partitiondrv_id);
    int partition_count = 0;
    DataLinkedList<blockdev::partition_info_t>partition_info_list;

    debug::out::printf("partitiondrv_id : %d\n" , partitiondrv_id);
    
    if(partition_driver == 0x00) return false; // no partition!
    debug::out::printf("Detecting partition...\n");
    partition_info_list.init();
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

void storage_system::add_logical_device(blockdev::block_device_driver *driver , blockdev::block_device *device , const blockdev::partition_info_t partition_info) {
    blockdev::block_device *new_logical_device;
    logical_block_device_driver *logical_driver = new logical_block_device_driver;
    logical_driver->set_super_driver(driver);
    new_logical_device->device_driver = logical_driver;
    new_logical_device = blockdev::create_empty_device();
    new_logical_device->id = device->id;
    memcpy(&new_logical_device->resource , &device->resource , sizeof(device_resources));
    memcpy(&new_logical_device->storage_info.partition_info , &partition_info , sizeof(blockdev::partition_info_t));
    
    new_logical_device->storage_info.partition_id = device->storage_info.logical_block_devs->register_object(new_logical_device);
}

blockdev::block_device *storage_system::get_physical_super_device(blockdev::block_device *logical_device) {
    if(logical_device->storage_info.storage_type != blockdev::logical) return 0x00;
    logical_block_device_driver *driver = (logical_block_device_driver *)logical_device->device_driver;
    if(driver == 0x00) return 0x00;
    if(driver->super_driver == 0x00) return 0x00;

    return driver->super_driver->device_container->get_object(logical_device->id);
}