#include <drivers/partition_driver.hpp>

max_t storage_system::register_partition_driver(storage_system::PartitionDriver *partition_driver) {
    PartitionDriverContainer *partitiondrv_container = PartitionDriverContainer::get_self();
    return partitiondrv_container->register_object(partition_driver);
}

/// @brief Identify what partition driver block device uese
/// @param device pointer of the block device
/// @return id of using partition driver
max_t storage_system::identify_partition_driver(blockdev::block_device *device) {
    PartitionDriverContainer *partitiondrv_container = PartitionDriverContainer::get_self();
    for(max_t id = 0; id < partitiondrv_container->max_count; id++) {
        if(partitiondrv_container->object_container[id].object == 0x00) continue;

        if(partitiondrv_container->object_container[id].object->identify(device) == true) {
            device->storage_info.partition_driver_id = id;
            return id;
        }
    }
    device->storage_info.partition_driver_id = INVALID;
    return INVALID;
}

storage_system::PartitionDriver *storage_system::get_partition_identifier(blockdev::block_device *device) {
    return PartitionDriverContainer::get_self()->get_object(device->storage_info.partition_driver_id);
}

storage_system::PartitionDriver *storage_system::get_partition_identifier(max_t identifier_id) {
    if(identifier_id == INVALID) return 0x00;
    return PartitionDriverContainer::get_self()->get_object(identifier_id);
}