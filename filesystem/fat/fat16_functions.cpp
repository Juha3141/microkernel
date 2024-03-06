#include "fat16.hpp"

void fat16::write_vbr(fat16::fat16_vbr_t *vbr , blockdev::block_device *device , const char *oem_id , const char *volume_label , const char *fs) {
    dword total_sector_count = device->geometry.lba_total_block_count;
    dword root_dir_sector_count;
    vbr->bytes_per_sector = device->geometry.block_size;
    if(total_sector_count > 65535) {
        vbr->total_sectors_16 = 0;
        vbr->total_sectors_32 = total_sector_count;
    }
    else {
        vbr->total_sectors_16 = (unsigned short)total_sector_count;
        vbr->total_sectors_32 = 0;
    }
    vbr->fat_count = 2;
    vbr->media_type = 0xF8; // Fixed : 0xF8 , Movable : 0xF0
    vbr->hidden_sectors = 0; // Hidden Sectors : 0
    vbr->jump_code[0] = 0xEB;
    vbr->jump_code[1] = 0x3C;
    vbr->jump_code[2] = 0x90;
    memcpy(vbr->oem_id , oem_id , 8);
    memcpy(vbr->volume_label , volume_label , 11);
    memcpy(vbr->fs_type , fs , 8);
    vbr->reserved = 0;
    vbr->serial_num = 0x31415926;
    vbr->boot_signature = 0x27;
    vbr->sectors_per_track = device->geometry.sector;
    vbr->number_of_heads = device->geometry.head;
    // determine cluster size
    vbr->sectors_per_cluster = 0;
    if(total_sector_count <= 32768) { // 7MB~16MB : 2KB
        vbr->sectors_per_cluster = 4;
    }
    if((total_sector_count > 32768) && (total_sector_count < 65536)) { // 17MB~32MB : 512B
        vbr->sectors_per_cluster = 1;
    }
    if((total_sector_count >= 65536) && (total_sector_count < 131072)) { // 33MB~64MB : 1KB
        vbr->sectors_per_cluster = 2;
    }
    if((total_sector_count >= 131072) && (total_sector_count < 262144)) { // 65~128MB : 2KB
        vbr->sectors_per_cluster = 4;
    }
    if((total_sector_count >= 262144) && (total_sector_count < 524288)) { // 129MB~256MB : 4KB
        vbr->sectors_per_cluster = 8;
    }
    if((total_sector_count >= 524288) && (total_sector_count < 1048576)) { // 257~512MB : 8KB
        vbr->sectors_per_cluster = 16;
    }
    if((total_sector_count >= 1048576) && (total_sector_count < 2097152)) { // 513~1024MB : 16KB
        vbr->sectors_per_cluster = 32;
    }
    if((total_sector_count >= 2097152) && (total_sector_count < 4194304)) { // 1025~2048MB : 32KB
        vbr->sectors_per_cluster = 64;
    }
    if((total_sector_count >= 4194304) && (total_sector_count < 8388608)) { // 2049~4096MB : 64KB
        vbr->sectors_per_cluster = 128;
    }
    vbr->reserved_sector_count = vbr->sectors_per_cluster*1;
    // determine root directory entry count
    // used some of code from :
    // https://github.com/Godzil/dosfstools/blob/master/src/mkdosfs.c, line 603
    switch(total_sector_count) {
        case 720: // 5.25" - 360KB
            vbr->root_dir_entry_count = 112;
            vbr->sectors_per_cluster = 2;
            vbr->media_type = 0xFD;
            break;
        case 2400: // 5.25" - 1200KB
            vbr->root_dir_entry_count = 224;
            vbr->sectors_per_cluster = 2;
            vbr->media_type = 0xF9;
            break;
        case 1440: // 3.5" - 720KB
            vbr->root_dir_entry_count = 112;
            vbr->sectors_per_cluster = 2;
            vbr->media_type = 0xF9;
            break;
        case 5760: // 3.5" - 2880KB
            vbr->root_dir_entry_count = 224;
            vbr->sectors_per_cluster = 2;
            vbr->media_type = 0xF0;
            break;
        case 2880: // 3.5" - 1440KB
            vbr->root_dir_entry_count = 224;
            vbr->sectors_per_cluster = 2;
            vbr->media_type = 0xF0;
            break;
        default:
            vbr->root_dir_entry_count = 512; // Root directory entry size : 32 sectors
            break;
        // If I remove this the code works fine.. in WINDOWS.
    }
    // vbr->FATSize16 = ((total_sector_count/vbr->sectors_per_cluster)*sizeof(unsigned short)/Storage->PhysicalInfo.Geometry.BytesPerSector)-1;
    root_dir_sector_count = ((vbr->root_dir_entry_count*32)+(vbr->bytes_per_sector-1))/vbr->bytes_per_sector;
    vbr->fat_size_16 = 128;
}

void fat16::get_ginfo(fat::general_fat_info &ginfo , fat16_vbr_t *vbr) {
    ginfo.fat_area_loc = get_fat_area_loc(vbr);
    ginfo.fat_size = vbr->fat_size_16;

    ginfo.cluster_size = sizeof(unsigned short);
    
    ginfo.data_area_loc = get_data_area_loc(vbr);

    ginfo.root_dir_loc = get_root_directory(vbr);
    ginfo.root_dir_size = get_root_directory_size(vbr);
    
    ginfo.vbr = vbr;
}

dword fat16::get_fat_area_loc(fat16::fat16_vbr_t *vbr) { return vbr->reserved_sector_count; }

/// @brief Get sector number of the root directory
/// @param vbr VBR of the sector
/// @return Sector number of the root directory
dword fat16::get_root_directory(fat16::fat16_vbr_t *vbr) { return vbr->reserved_sector_count+(vbr->fat_size_16*vbr->fat_count); }

/// @brief Get size of the root directory "in sector"
/// @param VBR VBR of the storage
/// @return Sector size of the root directory
dword fat16::get_root_directory_size(fat16::fat16_vbr_t *vbr) { return ((((vbr->root_dir_entry_count*32))/vbr->bytes_per_sector)); }

/// @brief Get sector number of the data area
/// @param VBR VBR of the storage
/// @return Sector number of the data area
dword fat16::get_data_area_loc(fat16::fat16_vbr_t *vbr) {
    return fat16::get_root_directory(vbr)+fat16::get_root_directory_size(vbr);
}

// Return absolute size of directory(which means it returns size of directory "in bytes")
// Also writes cluster size of the directory
// -> ERROR

/*
/// @brief Get location of the directory that consists the file
/// @param Storage Pointer of the storage
/// @param FileName Full file name
/// @return Return *sector location* of the directory
dword fat16::get_directory_location(blockdev::block_device *device , dword diectory_loc , const char *file_name) {
    int i;
    int directory_loc;
    int directory_count = 0;
    char **directory_list;
    sfn_entry_t *sfn_entry;
    
    common_vbr_t *vbr;

    DirectoryLocation = GetRootDirectoryLocation(&(VBR));
    DirectoryCount = FileSystem::GetDirectoryCount(FileName);
    if(DirectoryCount == 0) {
        return GetRootDirectoryLocation(&(VBR));
    }
    DirectoryList = (char **)MemoryManagement::Allocate((DirectoryCount+1)*sizeof(char *));
    FileSystem::ParseDirectories(FileName , DirectoryList);
    for(i = 0; i < DirectoryCount; i++) {
        if((GetSFNEntry(Storage , DirectoryLocation , DirectoryList[i] , &(SFNEntry)) == false)||(SFNEntry.Attribute != 0x10)) {
            // free
            for(i = 0; i < DirectoryCount; i++) {
                MemoryManagement::Free(DirectoryList[i]);
            }
            MemoryManagement::Free(DirectoryList);
            return 0xFFFFFFFF;
        }
        DirectoryLocation = ClusterToSector(((SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow) , &(VBR));
    }
    for(i = 0; i < DirectoryCount; i++) {
        MemoryManagement::Free(DirectoryList[i]);
    }
    MemoryManagement::Free(DirectoryList);
    return DirectoryLocation;
}

*/