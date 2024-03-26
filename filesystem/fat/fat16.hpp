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
    
    struct fat16_driver : file_device_driver {
        bool check(blockdev::block_device *device);
        bool get_root_directory(physical_file_location &file_loc);

        bool create(const general_file_name file_name , word file_type);

        file_info *get_file_handle(const general_file_name file_name);

        bool remove(const general_file_name file_name);

        bool rename(const general_file_name file_name , const char *new_file_name);
        bool move(const general_file_name file_name , file_info *new_directory);

        bool read_block(file_info *file , max_t file_block_addr , void *buffer);
        bool write_block(file_info *file , max_t file_block_addr , const void *buffer);
        max_t allocate_new_block(file_info *file , max_t &unit_size);
        max_t get_phys_block_address(file_info *file , max_t linear_block_addr);

        int read_directory(file_info *file , max_t cursor);
    };

    void register_driver(void);

    void write_vbr(fat16_vbr_t *vbr , blockdev::block_device *device , const char *oem_id , const char *volume_label , const char *fs);
    
    void get_ginfo(fat::general_fat_info_t &ginfo , fat16_vbr_t *vbr);

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