#include <integrated/mbr.hpp>

bool MBRPartitionDriver::identify(storagedev::storage_device *device) {
    int i;
	mbr_partition_table partition_table;
    if((device->device_driver == 0x00)
    ||(device->geomtery.bytes_per_sector != 512)) return false;

    if(device->device_driver->read_sector(device , 0 , 1 , &partition_table) != 512) return false;
    if(partition_table.signature != 0xAA55) return false;
    for(i = 0; i < 4; i++) {
        // If storage is logical storage, and it's not extended partition -> No partition found.
        if((device->storage_type == storagedev::logical)
	    && (partition_table.entries[i].partition_type != 0x0F) && (partition_table.entries[i].partition_type != 0x05)) return false;
        // GPT
		if(partition_table.entries[i].partition_type == 0xEE) return false;

		// MBR
        if(partition_table.entries[i].partition_type != 0x00) return true;
    }
    return false;
}

int MBRPartitionDriver::get_partitions_count(storagedev::storage_device *device) {
	int partition_count = 0;
	mbr_partition_table partition_table;

    if(device->device_driver->read_sector(device , 0 , 1 , &partition_table) != 512) return 0;
    for(int i = 0; i < 4; i++) {
        if(partition_table.entries[i].partition_type != 0x00) {
			partition_count++;
        }
    }
    debug::out::printf(DEBUG_INFO ,"mbrdrv::get_part_cnt" , "device %s%d : %d partitions\n" , device->device_driver->driver_name , device->id , partition_count);
    return partition_count;
}

int MBRPartitionDriver::get_partitions_list(storagedev::storage_device *device , DataLinkedList<storagedev::partition_info> &partition_info_list) {
	int partition_count = 0;
	mbr_partition_table partition_table;

    if(device->device_driver->read_sector(device , 0 , 1 , &partition_table) != 512) return 0;
    for(int i = 0; i < 4; i++) {
        if(partition_table.entries[i].partition_type != 0x00) {
			DataLinkedList<storagedev::partition_info>::node_s *node = partition_info_list.register_data_rear();

            node->data.physical_sector_start = partition_table.entries[i].starting_lba;
            node->data.physical_sector_end = partition_table.entries[i].starting_lba+partition_table.entries[i].size_lba;
            node->data.bootable = (partition_table.entries[i].bootable_flag == 0x00);
            partition_count++;
        }
    }
    debug::out::printf(DEBUG_INFO ,"mbrdrv::get_part_cnt" , "device %s%d : %d partitions\n" , device->device_driver->driver_name , device->id , partition_count);
    return partition_count;
}

bool MBRPartitionDriver::create_partition(storagedev::storage_device *device , storagedev::partition_info partition) {
	return false;
}

bool MBRPartitionDriver::remove_partition(storagedev::storage_device *device , storagedev::partition_info partition) {
	return false;
}

bool MBRPartitionDriver::modify_partition(storagedev::storage_device *device , storagedev::partition_info old_partition , storagedev::partition_info new_partition_info) {
	return false;
}