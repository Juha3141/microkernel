#if 0

typedef struct fat32_vbr_s {
    byte jump_code[3];
    byte oem_id[8];
    word bytes_per_sector;
    byte sectors_per_cluster;
    word reserved_sector_count;
    byte fat_count; // 2
    word root_dir_entry_count; // 2880
    word total_sectors_16;
    byte media_type;
    word fat_size_16; // ((total_sector/sectors_per_clusters)*2/*fat16 : 2, fat32 : 4*/)/bytes_per_sector (certified)
    word sectors_per_track;
    word number_of_heads;
    dword hidden_sectors;
    dword total_sectors_32;

   /******* fat32 *********/
    dword fat32_size;
    word ext_flags;
    word file_system_version;
    dword root_directory_cluster;
    word file_system_info;
    word backup_bootsector;
    byte reserved[12];
    byte int0x13_drv_num;
    byte reserved1;
    byte boot_signature;
    dword serial_num;
    byte volume_label[11];
    byte fs_type[8];
}fat32_vbr_t;

#endif