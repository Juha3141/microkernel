#include "mbr.hpp"

bool MBRPartitionDriver::identify(block_device *device) {
    int i;
	mbr_partition_table partition_table;
    if((device->driver == 0x00)
    ||(device->geometry.block_size != 512)) return false;

    if(device->driver->read(device , 0 , 1 , &partition_table) != 512) return false;
    if(partition_table.signature != 0xAA55) return false;
    for(i = 0; i < 4; i++) {
        // If storage is logical storage, and it's not extended partition -> No partition found.
        if((device->storage_info.storage_type == storage_logical)
	    && (partition_table.entries[i].partition_type != 0x0F) && (partition_table.entries[i].partition_type != 0x05)) return false;
        // GPT
		if(partition_table.entries[i].partition_type == 0xEE) return false;

		// MBR
        if(partition_table.entries[i].partition_type != 0x00) return true;
    }
    return false;
}

int MBRPartitionDriver::get_partitions_count(block_device *device) {
	int partition_count = 0;
	mbr_partition_table partition_table;

    if(device->driver->read(device , 0 , 1 , &partition_table) != 512) return 0;
    for(int i = 0; i < 4; i++) {
        if(partition_table.entries[i].partition_type != 0x00) {
			partition_count++;
        }
    }
    return partition_count;
}

int MBRPartitionDriver::get_partitions_list(block_device *device , LinkedList<partition_info_t> &partition_info_list) {
	int partition_count = 0;
	mbr_partition_table partition_table;

    if(device->driver->read(device , 0 , 1 , &partition_table) != 512) return 0;
    for(int i = 0; i < 4; i++) {
        if(partition_table.entries[i].partition_type != 0x00) {

            partition_info_list.add_rear({
            .physical_sector_start = partition_table.entries[i].starting_lba, 
            .physical_sector_end = partition_table.entries[i].starting_lba+partition_table.entries[i].size_lba, 
            .bootable = (partition_table.entries[i].bootable_flag == 0x00), 
            });
            
            partition_count++;
        }
    }
    return partition_count;
}

// not implemented
bool MBRPartitionDriver::create_partition(block_device *device , partition_info_t partition) {
	return false;
}

bool MBRPartitionDriver::remove_partition(block_device *device , partition_info_t partition) {
	return false;
}

bool MBRPartitionDriver::modify_partition(block_device *device , partition_info_t old_partition , partition_info_t new_partition_info) {
	return false;
}

static void init_mbr_partition_driver(void) { storage_system::register_partition_driver(new MBRPartitionDriver , "mbr"); }

REGISTER_FS_DRIVER(init_mbr_partition_driver)