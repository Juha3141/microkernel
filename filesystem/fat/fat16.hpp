#ifndef _FAT16_HPP_
#define _FAT16_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/file_system_driver.hpp>

#include "fat.hpp"

namespace fat16 {
    typedef struct fat16_vbr_s {
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

        /******* fat16 *********/
        byte int0x13_drv_num;
        byte reserved;
        byte boot_signature;
        dword serial_num;
        byte volume_label[11];
        byte fs_type[8];
    }fat16_vbr_t;

    void register_driver(void);

    void write_vbr(fat16_vbr_t *vbr , blockdev::block_device *device , const char *oem_id , const char *volume_label , const char *fs);
    
    void get_ginfo(fat::general_fat_info &ginfo , fat16_vbr_t *vbr);

    dword get_fat_area_loc(fat16_vbr_t *vbr);
    dword get_root_directory(fat16_vbr_t *vbr);
    dword get_root_directory_size(fat16_vbr_t *vbr);
    dword get_data_area_loc(fat16_vbr_t *vbr);
    // File name whatever
    int get_file_name_from_lfn(char *file_name , lfn_entry_t *entries);
    
    // deprecated??
    dword get_directory_location(blockdev::block_device *device , const char *file_name);
};

#endif