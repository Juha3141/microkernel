#ifndef _GPT_HPP_
#define _GPT_HPP_

#include <interface_type.hpp>
#include <drivers/partition_driver.hpp>

#define GPT_PARTITION_SYSTEM 	       0x00
#define GPT_PARTITION_EFI_FIRMWARE     0x01
#define GPT_PARTITION_LEGACY_BOOTABLE  0x02
#define GPT_PARTITION_READ_ONLY 	   0x60
#define GPT_PARTITION_SHADOW_COPY      0x61
#define GPT_PARTITION_HIDDEN	 	   0x62
#define GPT_PARTITION_NO_DRIVE_LETTEER 0x63

struct gpt_header {
	unsigned char signature[8];
	unsigned int revision;
	unsigned int header_size;
	unsigned int crc32_value;
	unsigned int reserved1;
	unsigned long gpt_header_lba;
	unsigned long backup_gpt_header_lba;
	unsigned long available_partition_lba;
	unsigned long ending_available_partition_lba;
	unsigned char disk_guid[16];
	unsigned long partition_table_entry_lba;
	unsigned int partition_table_entry_count;
	unsigned int partition_table_entry_size;
	unsigned partition_table_cr32_value;
	unsigned char reserved2[420];
};
struct gpt_partition_table_entry {
	unsigned char partition_type_guid[16];
	unsigned char unique_partition_guid[16];
	unsigned long start_address_lba;
	unsigned long end_address_lba;
	unsigned long attribute_flag;
	char partition_name[72];
};

struct GPTPartitionDriver : blockdev::PartitionDriver {
    bool identify(blockdev::block_device *device) override;
	int get_partitions_count(blockdev::block_device *device) override;
    int get_partitions_list(blockdev::block_device *device , DataLinkedList<blockdev::partition_info> &partition_info_list) override;
    bool create_partition(blockdev::block_device *device , blockdev::partition_info partition) override;
    bool remove_partition(blockdev::block_device *device , blockdev::partition_info partition) override;
    bool modify_partition(blockdev::block_device *device , blockdev::partition_info old_partition , blockdev::partition_info new_partition_info) override;
	
	const char *driver_name = "gpt";
};

#endif