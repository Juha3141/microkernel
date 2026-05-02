#include <kernel/vfs/partition_driver.hpp>

storage_system::PartitionDriverContainer *partitiondrv_container;

void storage_system::init(void) {
    partitiondrv_container = memory::new_global_object<storage_system::PartitionDriverContainer>();
}

max_t storage_system::register_partition_driver(storage_system::PartitionDriver *partition_driver , const char *driver_name) {
    strcpy(partition_driver->driver_name , driver_name);
    return partitiondrv_container->add(partition_driver);
}

/// @brief Identify what partition driver block device uese
/// @param device pointer of the block device
/// @return id of using partition driver
max_t storage_system::identify_partition_driver(block_device *device) {
    for(max_t id = 0; id < partitiondrv_container->get_max_size(); id++) {
        if((*partitiondrv_container->container[id]) == 0x00) continue;

        if((*partitiondrv_container->container[id])->identify(device) == true) {
            device->storage_info.partition_driver_id = id;
            return id;
        }
    }
    device->storage_info.partition_driver_id = INVALID;
    return INVALID;
}

storage_system::PartitionDriver *storage_system::get_partition_identifier(block_device *device) {
    return partitiondrv_container->get(device->storage_info.partition_driver_id);
}

storage_system::PartitionDriver *storage_system::get_partition_identifier(max_t identifier_id) {
    if(identifier_id == INVALID) return 0x00;
    return partitiondrv_container->get(identifier_id);
}