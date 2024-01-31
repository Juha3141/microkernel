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

struct MBRPartitionDriver : storagedev::PartitionDriver {
    bool identify(storagedev::storage_device *device) override;
	int get_partitions_count(storagedev::storage_device *device) override;
    int get_partitions_list(storagedev::storage_device *device , DataLinkedList<storagedev::partition_info> &partition_info_list) override;
    bool create_partition(storagedev::storage_device *device , storagedev::partition_info partition) override;
    bool remove_partition(storagedev::storage_device *device , storagedev::partition_info partition) override;
    bool modify_partition(storagedev::storage_device *device , storagedev::partition_info old_partition , storagedev::partition_info new_partition_info) override;

	const char *driver_name = "mbr";
};

#endif