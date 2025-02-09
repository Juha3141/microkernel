#include "gpt.hpp"

#include <kernel/debug.hpp>

bool GPTPartitionDriver::identify(blockdev::block_device *device) {
    byte buffer[512];
    if((device->device_driver == 0x00)
    ||(device->geometry.block_size != 512)) return false;
    
    gpt_header *header = (gpt_header *)buffer;
    if(device->device_driver->read(device , 1 , 1 , buffer) != 512) return false;
    if(memcmp(header->signature , "EFI PART" , 8) != 0) {
        return false;
    }
    debug::out::printf(DEBUG_INFO , "gptdrv::identify" , "device %s%d : GPT detected" , device->device_driver->driver_name , device->id);
    return true;
}

int GPTPartitionDriver::get_partitions_count(blockdev::block_device *device) {
    int partition_count = 0;
    dword table_entry_size;
    unsigned char buffer[512];
    gpt_header *header = (gpt_header *)buffer;
    gpt_partition_table_entry *partition_entry;
    
    if(device->device_driver->read(device , 1 , 1 , buffer) != 1) return 0;
    table_entry_size = header->partition_table_entry_count*sizeof(gpt_partition_table_entry);
    
    table_entry_size = (table_entry_size%512 == 0) ? ((dword)(table_entry_size/512)+1)*512 : table_entry_size;
    partition_entry = (gpt_partition_table_entry *)memory::pmem_alloc(table_entry_size);
    
    device->device_driver->read(device , header->partition_table_entry_lba ,  table_entry_size/512 , partition_entry);
    for(int i = 0; i < header->partition_table_entry_count; i++) {
        if((partition_entry[i].partition_type_guid[0] == 0) && (partition_entry[i].partition_type_guid[1] == 0)
        && (partition_entry[i].partition_type_guid[2] == 0) && (partition_entry[i].partition_type_guid[3] == 0)) {
            continue;
        }
        partition_count++;
    }
    
    memory::pmem_free(partition_entry);
    return partition_count;
}

int GPTPartitionDriver::get_partitions_list(blockdev::block_device *device , DataLinkedList<blockdev::partition_info_t> &partition_info_list) {
    int i;
    unsigned char buffer[512];
    int partition_count = 0;
    dword table_entry_size;
    gpt_header *header = (gpt_header *)buffer;
    gpt_partition_table_entry *partition_entry;

    if(device->device_driver->read(device , 1 , 1 , buffer) != 1) return 0;
    table_entry_size = header->partition_table_entry_count*sizeof(gpt_partition_table_entry);
    
    table_entry_size = (table_entry_size%512 == 0) ? ((dword)(table_entry_size/512)+1)*512 : table_entry_size;
    partition_entry = (gpt_partition_table_entry *)memory::pmem_alloc(table_entry_size);
    
    device->device_driver->read(device , header->partition_table_entry_lba ,  table_entry_size/512 , partition_entry);
    for(i = 0; i < header->partition_table_entry_count; i++) {
        if((partition_entry[i].partition_type_guid[0] == 0) && (partition_entry[i].partition_type_guid[1] == 0)
        && (partition_entry[i].partition_type_guid[2] == 0) && (partition_entry[i].partition_type_guid[3] == 0)) {
            continue;
        }
        DataLinkedList<blockdev::partition_info_t>::node_s *node = partition_info_list.register_data_rear();
        node->data.physical_sector_start = partition_entry[i].start_address_lba;
        node->data.physical_sector_end = partition_entry[i].end_address_lba;
        node->data.bootable = (partition_entry[i].attribute_flag == GPT_PARTITION_LEGACY_BOOTABLE);
        partition_count++;
    }

    memory::pmem_free(partition_entry);
    return partition_count;
}

bool GPTPartitionDriver::create_partition(blockdev::block_device *device , blockdev::partition_info_t partition) {
    // Not implemented yet
    return false;
}

bool GPTPartitionDriver::remove_partition(blockdev::block_device *device , blockdev::partition_info_t partition) {
    // Not implemented yet
    return false;
}

bool GPTPartitionDriver::modify_partition(blockdev::block_device *device , blockdev::partition_info_t old_partition , blockdev::partition_info_t new_partition_info) {
    return false;
}

static void init_gpt_partition_driver(void) {
    storage_system::register_partition_driver(new GPTPartitionDriver);
}

REGISTER_DRIVER(init_gpt_partition_driver)