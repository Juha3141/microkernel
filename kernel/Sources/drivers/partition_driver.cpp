#include <drivers/partition_driver.hpp>

max_t blockdev::register_partition_driver(blockdev::PartitionDriver *partition_driver) {
    PartitionDriverContainer *partitiondrv_container = PartitionDriverContainer::get_self();
    return partitiondrv_container->register_object(partition_driver);
}

/// @brief Identify what partition driver block device uese
/// @param device pointer of the block device
/// @return id of using partition driver
max_t blockdev::identify_partition_driver(blockdev::block_device *device) {
    PartitionDriverContainer *partitiondrv_container = PartitionDriverContainer::get_self();
    for(max_t id = 0; id < partitiondrv_container->max_count; id++) {
        if(partitiondrv_container->object_container[id].object->identify(device) == true) {
            device->partition_driver_id = id;
            return id;
        }
    }
    device->partition_driver_id = INVALID;
    return INVALID;
}

blockdev::PartitionDriver *blockdev::get_partition_identifier(blockdev::block_device *device) {
    if(device->partition_driver_id == INVALID) {
        return 0x00;
    }
    return PartitionDriverContainer::get_self()->get_object(device->partition_driver_id);
}