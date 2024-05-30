#include "fat16.hpp"
#include "fat12.hpp"

#include <kernel/mem/kmem_manager.hpp>

//////////////// FAT16 ////////////////

bool fat16::fat16_driver::check(blockdev::block_device *device) {
    byte buffer[512];
    fat16::fat16_vbr_t *boot_sector;
    debug::out::printf("device->geometry.block_size : %d\n" , device->geometry.block_size);
    if(device->geometry.block_size != 512) return false;
    debug::out::printf("checking whether device is fat16...\n");
    if(device->device_driver->read(device , 0 , 1 , buffer) != 512) return false;
    
    boot_sector = (fat16::fat16_vbr_t *)buffer;
    byte jump_code[3] = {0xEB , 0x3C , 0x90};
    return (memcmp(boot_sector->fs_type , "FAT16" , 5) == 0) && (memcmp(boot_sector->jump_code , (void *)&jump_code , 3) == 0);
}

bool fat16::fat16_driver::create(const general_file_name file_name , word file_type) {
    byte data[512];
    char sfn_name[13];
    sfn_entry_t sfn_entry;
    dword empty_cluster_loc;

    physical_file_location *rootdir_file_loc = fsdev::get_physical_loc_info(file_name.root_directory);

    // get the ginfo and vbr
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(rootdir_file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

debug::push_function("fat16::fat16_driver::create");

    debug::out::printf("fat area location : %d\n" , ginfo.fat_area_loc);
    debug::out::printf("fat area size     : %d\n" , ginfo.fat_size);
    empty_cluster_loc = fat::find_first_empty_cluster(rootdir_file_loc->block_device , ginfo); // Get the first usable cluster(Empty cluster)
    
    debug::out::printf("empty_cluster_loc : %d\n" , empty_cluster_loc);

debug::pop_function();

    fat::write_cluster_info(rootdir_file_loc->block_device , empty_cluster_loc , 0xFFFF , ginfo);
    // Initialize SFN Entry
    memset(&sfn_entry , 0 , sizeof(sfn_entry_t));
    // Create SFN Name (Ex : HELLOW~1.TXT)
    fat::create_sfn_name(sfn_name , file_name.file_name , 1); // number : default to 1
    strncpy((char *)sfn_entry.file_name , sfn_name , 11);

    // Write data to SFN Entry
    sfn_entry.file_size = 0;     // Set file size to zero for now.
    sfn_entry.attribute = 0x20; // normal file
    sfn_entry.starting_cluster_low = empty_cluster_loc & 0xFFFF;
    sfn_entry.starting_cluster_high = (empty_cluster_loc << 16) & 0xFFFF;

    // Write file entry to directory address, LFN comes before SFN.
    fat::write_lfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , file_name.file_name , ginfo);
    fat::write_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , &sfn_entry , ginfo);
    return true;
}

file_info *fat16::fat16_driver::get_file_handle(const general_file_name file_name) {
    physical_file_location *rootdir_file_loc = fsdev::get_physical_loc_info(file_name.root_directory);
    // The juxtaposition of namespaces
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(rootdir_file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

    sfn_entry_t sfn_entry;
    if(fat::get_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , file_name.file_name , &sfn_entry , ginfo) == false) {
        debug::out::printf("sfn entry not found!\n");
        return 0x00;
    }
    debug::out::printf("%s\n" , sfn_entry.file_name);
    max_t cluster = (sfn_entry.starting_cluster_high << 16)|(sfn_entry.starting_cluster_low);
    debug::out::printf("file start cluster : %dcluster (%dsector)\n" , cluster , fat::cluster_to_sector(cluster , ginfo));
    while(cluster != ginfo.invalid_cluster_info) {
        cluster = fat::find_next_cluster(rootdir_file_loc->block_device , cluster , ginfo);
        debug::out::printf("next : %dcluster (%dsector)\n" , cluster , fat::cluster_to_sector(cluster , ginfo));
    }
    
    file_info *new_file = fat::write_file_info_by_sfn(rootdir_file_loc , file_name.file_name , sfn_entry , ginfo);
    return new_file;
}

bool fat16::fat16_driver::remove(const general_file_name file_name) {
    physical_file_location *rootdir_file_loc = fsdev::get_physical_loc_info(file_name.root_directory);
    // The juxtaposition of namespaces
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(rootdir_file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

    sfn_entry_t sfn_entry;
    if(fat::get_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , file_name.file_name , &sfn_entry , ginfo) == false) {
        debug::out::printf("sfn entry not found!\n");
        return false;
    }

    dword starting_cluster_location = (sfn_entry.starting_cluster_high << 16)|sfn_entry.starting_cluster_low;
    dword cluster_count = sfn_entry.file_size/(vbr.sectors_per_cluster*vbr.bytes_per_sector)
                +(sfn_entry.file_size%(vbr.sectors_per_cluster*vbr.bytes_per_sector) != 0);
    
    char sfn_name[12] = {0 , };
    fat::create_sfn_name(sfn_name , file_name.file_name , 1);
    if(fat::mark_entry_removed(rootdir_file_loc->block_device , rootdir_file_loc->block_location , sfn_name , ginfo) == false) return false;
    
    dword cluster_ptr = starting_cluster_location;
    for(dword i = 0; i < cluster_count; i++) {
        dword next_cluster = fat::find_next_cluster(rootdir_file_loc->block_device , cluster_ptr , ginfo);
        fat::write_cluster_info(rootdir_file_loc->block_device , cluster_ptr , 0x00 , ginfo);
        cluster_ptr = next_cluster;
    }
    return true;
}

bool fat16::fat16_driver::rename(const general_file_name file_name , const char *new_name) {
    return false;
}

bool fat16::fat16_driver::move(const general_file_name file_name , file_info *new_directory) {
    return false;
}

max_t fat16::fat16_driver::get_cluster_size(file_info *file) {
    fat16::fat16_vbr_t vbr;
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    fat::get_vbr(file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));

    return vbr.sectors_per_cluster;
}

max_t fat16::fat16_driver::allocate_new_cluster_to_file(file_info *file) {
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    fat::get_vbr(file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);
    
    word cluster = fat::sector_to_cluster(file_loc->block_location , ginfo);
    while(1) {
        word next = fat::find_next_cluster(file_loc->block_device , cluster , ginfo);
        if(next == ginfo.invalid_cluster_info||next == 0x00) break;
        cluster = next;
    }
    word file_last_cluster = cluster;
    word new_cluster = fat::find_first_empty_cluster(file_loc->block_device , ginfo);
    word accuracy = fat::find_next_cluster(file_loc->block_device , new_cluster , ginfo);

    fat::write_cluster_info(file_loc->block_device , new_cluster , ginfo.invalid_cluster_info , ginfo);
    fat::write_cluster_info(file_loc->block_device , file_last_cluster , new_cluster , ginfo);

    max_t sz = vbr.bytes_per_sector*vbr.sectors_per_cluster;
    char zero_buffer[sz];
    memset(zero_buffer , 0 , sz);
    file_loc->block_device->device_driver->write(file_loc->block_device , fat::cluster_to_sector(new_cluster , ginfo) , vbr.sectors_per_cluster , zero_buffer);
    return fat::cluster_to_sector(new_cluster , ginfo);
}

bool fat16::fat16_driver::get_root_directory(physical_file_location &file_loc) {
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(file_loc.block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    file_loc.block_location = fat16::get_root_directory(&vbr);
    return true;
}

max_t fat16::fat16_driver::get_cluster_start_address(file_info *file , max_t linear_block_addr) {
    physical_file_location *file_loc;
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    max_t block_size;

    // Get the basic informations
    file_loc = fsdev::get_physical_loc_info(file);
    fat::get_vbr(file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    get_ginfo(ginfo , &vbr);

    block_size = file_loc->block_device->geometry.block_size;

    // search for the cluster
    max_t block_loc = file_loc->block_location;
    max_t cluster_loc = fat::sector_to_cluster(block_loc , ginfo);
    for(int i = 0; i < (linear_block_addr/(vbr.sectors_per_cluster)); i++) {
        cluster_loc = fat::find_next_cluster(file_loc->block_device , cluster_loc , ginfo);
        if(cluster_loc == ginfo.invalid_cluster_info) {
            return INVALID;
        }
    }
    // calculate the sector location
    max_t sector_loc = fat::cluster_to_sector(cluster_loc , ginfo);
    return sector_loc;
}

bool fat16::fat16_driver::apply_new_file_info(file_info *file , max_t new_size) {
    physical_file_location *rootdir_file_loc = fsdev::get_physical_loc_info(file->parent_dir);
    // The juxtaposition of namespaces
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(rootdir_file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

    sfn_entry_t sfn_entry;
    if(fat::get_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , file->file_name , &sfn_entry , ginfo) == false) {
        debug::out::printf("sfn entry not found!\n");
        return 0x00;
    }

    // change the contents of sfn entry
    sfn_entry.file_size = new_size;

    // apply
    char sfn_name[12];
    strncpy(sfn_name , (const char *)sfn_entry.file_name , 11);
    if(fat::rewrite_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , sfn_name , &sfn_entry , ginfo) == false) {
        return false;
    }
    return true;
}

int fat16::fat16_driver::read_directory(file_info *file , ObjectLinkedList<file_info> &file_list) {
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    fat::general_fat_info_t ginfo;
    fat16::fat16_vbr_t vbr;

    fat::get_vbr(file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

    return fat::get_file_list(file_loc , file_list , ginfo);
}

////////////////// FAT12 //////////////////

bool fat12::fat12_driver::check(blockdev::block_device *device) {
    byte buffer[512];
    fat16::fat16_vbr_t *boot_sector;
    debug::out::printf("device->geometry.block_size : %d\n" , device->geometry.block_size);
    if(device->geometry.block_size != 512) return false;
    debug::out::printf("checking whether device is fat12...\n");
    if(device->device_driver->read(device , 0 , 1 , buffer) != 512) return false;
    
    boot_sector = (fat16::fat16_vbr_t *)buffer;
    byte jump_code[3] = {0xEB , 0x3C , 0x90};
    return (memcmp(boot_sector->fs_type , "FAT12" , 5) == 0) && (memcmp(boot_sector->jump_code , (void *)&jump_code , 3) == 0);
}

void fat12::register_driver(void) { fsdev::register_driver(new fat12_driver , "fat12"); }

////////////////// FAT16 //////////////////

void fat16::register_driver(void) { fsdev::register_driver(new fat16_driver , "fat16"); }

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

void fat16::get_ginfo(fat::general_fat_info_t &ginfo , fat16_vbr_t *vbr) {
    ginfo.fat_area_loc = get_fat_area_loc(vbr);
    ginfo.fat_size = vbr->fat_size_16;

    ginfo.fat_type = GINFO_FAT_TYPE_16;
    ginfo.invalid_cluster_info = 0xFFFF;
    if(memcmp(vbr->fs_type , "FAT12" , 5) == 0) {
        ginfo.fat_type = GINFO_FAT_TYPE_12;
        ginfo.invalid_cluster_info = 0xFFF;
    }
    
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