/**
 * @file fat.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief General functions for fat file system
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho.
 * 
 */

// Original Code : https://github.com/Juha3141/OS/blob/master/Kernel/IntegratedDriver/Sources/FAT16.cpp

#ifndef _FAT_HPP_
#define _FAT_HPP_

#include <kernel/essentials.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>

#define FAT_ATTRIBUTE_READONLY    0x01
#define FAT_ATTRIBUTE_HIDDEN      0x02
#define FAT_ATTRIBUTE_SYSTEM      0x04
#define FAT_ATTRIBUTE_VOLUMELABEL 0x08
#define FAT_ATTRIBUTE_LFN         0x0F
#define FAT_ATTRIBUTE_DIRECTORY   0x10
#define FAT_ATTRIBUTE_SYSTEMDIR   0x16
#define FAT_ATTRIBUTE_NORMAL      0x20

#define FAT_FILENAME_REMOVED      0xE5

typedef struct common_vbr_s {
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
}common_vbr_t;

typedef struct sfn_entry_s {
    byte file_name[8];
    byte extension[3];
    byte attribute;
    word reserved;
    
    word created_time;
    word created_date;
    
    word last_accessed_date;
    
    word starting_cluster_high;
    
    word last_written_time;
    word last_written_date;
    
    word starting_cluster_low;

    dword file_size;
}sfn_entry_t;

typedef struct lfn_entry_s {
    byte seq_number;
    word file_name_1[5];
    byte attribute;
    byte reserved;
    byte checksum;
    word file_name_2[6];
    word first_cluster_low;
    word file_name_3[2];
}lfn_entry_t;

#define GINFO_FAT_TYPE_12 1
#define GINFO_FAT_TYPE_16 2
#define GINFO_FAT_TYPE_32 3

namespace fat {
    void get_vbr(blockdev::block_device *device , void *vbr , int vbr_sz);

    typedef struct general_fat_info_s {
        /* fat12 : 1
         * fat16 : 2
         * fat32 : 3
         */
        byte fat_type;

        dword fat_area_loc;
        dword root_dir_loc;
        dword root_dir_size;
        dword data_area_loc;
        void *vbr;

        dword fat_size;
        dword invalid_cluster_info;
    }general_fat_info_t;

    file_info *write_file_info_by_sfn(const physical_file_location *rootdir_loc , const char *file_name , sfn_entry_t &sfn_entry , fat::general_fat_info_t &ginfo);

    dword cluster_to_sector(dword cluster_num , general_fat_info_t &ginfo);
    dword sector_to_cluster(dword sector_num , general_fat_info_t &ginfo);

    dword read_cluster(blockdev::block_device *device , max_t cluster_number , max_t cluster_count , void *data , general_fat_info_t &ginfo);
    dword write_cluster(blockdev::block_device *device , max_t cluster_number , max_t cluster_count , void *data , general_fat_info_t &ginfo);
    
    dword find_next_cluster(blockdev::block_device *device , dword cluster , general_fat_info_t &ginfo);
    dword find_first_empty_cluster(blockdev::block_device *device , general_fat_info_t &ginfo);
    dword get_file_cluster_count(blockdev::block_device *device , dword cluster , general_fat_info_t &ginfo);
    void write_cluster_info(blockdev::block_device *device , dword cluster , max_t cluster_info , general_fat_info_t &ginfo);
    void extend_cluster(blockdev::block_device *device , dword end_cluster , dword extend_count , general_fat_info_t &ginfo);

    dword get_directory_info(blockdev::block_device *device , dword directory_sector_addr , general_fat_info_t &ginfo);

    // string operation
    int get_filename_from_lfn(char *file_name , lfn_entry_t *entries);
    void create_sfn_name(char *sfn_name , const char *lfn_name , int num);
    void create_volume_label_name(char *sfn_nam , const char *lfn_name);
    byte get_sfn_checksum(const char *sfn_name);

    bool write_sfn_entry(blockdev::block_device *device , dword directory_addr , sfn_entry_t *entry , general_fat_info_t &ginfo);
    bool write_lfn_entry(blockdev::block_device *device , dword directory_addr , const char *file_name , general_fat_info_t &ginfo);
    bool rewrite_sfn_entry(blockdev::block_device *device , dword directory_addr , const char *sfn_name , sfn_entry_t *new_sfn_entry , general_fat_info_t &ginfo);
    bool mark_entry_removed(blockdev::block_device *device , dword directory_addr , const char *sfn_name , general_fat_info_t &ginfo);
    bool get_sfn_entry(blockdev::block_device *device , dword directory_addr , const char *file_name , sfn_entry_t *destination , general_fat_info_t &ginfo);
    
    int get_file_list(physical_file_location *dir_location , ObjectLinkedList<file_info> &file_list , general_fat_info_t &ginfo);
};

#endif