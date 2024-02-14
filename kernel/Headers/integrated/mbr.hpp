#ifndef _MBR_HPP_
#define _MBR_HPP_

#include <interface_type.hpp>
#include <drivers/partition_driver.hpp>

#define MBR_ENTRY_OFFSET 446

struct mbr_partition_table_entry {
	byte bootable_flag;
	byte starting_chs[3];
	byte partition_type;
	byte ending_chs[3];
	dword starting_lba;
	dword size_lba;
};

struct mbr_partition_table {
	byte boot_code[MBR_ENTRY_OFFSET];
	mbr_partition_table_entry entries[4];
	word signature; // 0xAA55
};

struct MBRPartitionDriver : storage_system::PartitionDriver {
    bool identify(blockdev::block_device *device) override;
	int get_partitions_count(blockdev::block_device *device) override;
    int get_partitions_list(blockdev::block_device *device , DataLinkedList<blockdev::partition_info_t> &partition_info_list) override;
    bool create_partition(blockdev::block_device *device , blockdev::partition_info_t partition) override;
    bool remove_partition(blockdev::block_device *device , blockdev::partition_info_t partition) override;
    bool modify_partition(blockdev::block_device *device , blockdev::partition_info_t old_partition , blockdev::partition_info_t new_partition_info) override;

	const char *driver_name = "mbr";
};

#endif