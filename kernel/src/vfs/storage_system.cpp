#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/partition_driver.hpp>
#include <kernel/vfs/file_system_driver.hpp>
#include <kernel/vfs/file_system_driver.hpp>

bool storage_system::detect_partitions(block_device *device) {
    max_t partitiondrv_id = identify_partition_driver(device);
    PartitionDriver *partition_driver = get_partition_identifier(partitiondrv_id);
    int partition_count = 0;
    LinkedList<partition_info_t>partition_info_list;
    
    if(partition_driver == 0x00) return false; // no partition!
    debug::out::printf(DEBUG_TEXT , "Detecting partitions...\n");
    partition_info_list.init();
    partition_count = partition_driver->get_partitions_list(device , partition_info_list);
    
    device->storage_info.partition_driver_id = partitiondrv_id;

    device->storage_info.logical_block_devs = new FixedArray<block_device*>();
    device->storage_info.logical_block_devs->init(72); // maximum logical storages count
    
    LinkedList<partition_info_t>::node_s *ptr = partition_info_list.get_start_node();
    while(ptr != 0x00) {
        add_logical_device(device->driver , device , ptr->object);
        ptr = ptr->next;
    }
    return true; // partition
}

void storage_system::add_logical_device(block_device_driver *driver , block_device *device , const partition_info_t partition_info) {
    block_device *new_logical_device;
    logical_block_device_driver *logical_driver = new logical_block_device_driver;
    logical_driver->set_super_driver(driver);
    new_logical_device = create_empty_device<block_device>();

    new_logical_device->driver = logical_driver;
    new_logical_device->id = device->id;
    memcpy(&new_logical_device->resources , &device->resources , sizeof(device_resources));

    new_logical_device->geometry.lba_total_block_count = partition_info.physical_sector_end-partition_info.physical_sector_start;
    new_logical_device->geometry.block_size = device->geometry.block_size;
    new_logical_device->geometry.cylinder = device->geometry.cylinder;
    new_logical_device->geometry.head     = device->geometry.head;
    new_logical_device->geometry.sector   = device->geometry.sector;
    new_logical_device->geometry.is_chs   = device->geometry.is_chs;

    memcpy(&new_logical_device->storage_info.partition_info , &partition_info , sizeof(partition_info_t));
    new_logical_device->storage_info.storage_type = storage_logical;
    new_logical_device->storage_info.partition_id = device->storage_info.logical_block_devs->add(new_logical_device);

    debug::out::printf("Detecting partition for logical device %s%d, %d\n" , driver->driver_name , device->id , new_logical_device->storage_info.partition_id);
    new_logical_device->storage_info.fs_driver = fsdev::detect_fs(new_logical_device);
}

block_device *storage_system::get_physical_super_device(block_device *logical_device) {
    if(logical_device->storage_info.storage_type != storage_logical) return 0x00;
    logical_block_device_driver *driver = (logical_block_device_driver *)logical_device->driver;
    if(driver == 0x00) return 0x00;
    if(driver->super_driver == 0x00) return 0x00;

    return (block_device *)driver->super_driver->device_container->get(logical_device->id);
}