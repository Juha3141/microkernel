#include "fat16.hpp"

#include <kernel/kmem_manager.hpp>

static bool fat16_check(blockdev::block_device *device) {
    byte buffer[512];
    fat16::fat16_vbr_t *boot_sector;
    debug::out::printf("device->geometry.block_size : %d\n" , device->geometry.block_size);
    if(device->geometry.block_size != 512) return false;
    debug::out::printf("checking whether device is fat16...\n");
    if(device->device_driver->read(device , 0 , 1 , buffer) != 512) return false;
    
    boot_sector = (fat16::fat16_vbr_t *)buffer;
    return memcmp(boot_sector->fs_type , "FAT16" , 5) == 0;
}

static bool fat16_create(const general_file_name file_name , file_info *directory) {
    return false;
}

static file_info *write_file_info_by_sfn(const physical_file_location *rootdir_loc , const char *file_name , sfn_entry_t &sfn_entry , fat::general_fat_info &ginfo) {
    int file_type;
    switch(sfn_entry.attribute) {
        case FAT_ATTRIBUTE_READONLY:
            file_type = FILE_TYPE_READONLY;
            break;
        case FAT_ATTRIBUTE_HIDDEN:
            file_type = FILE_TYPE_HIDDEN;
            break;
        case FAT_ATTRIBUTE_SYSTEM:
            file_type = FILE_TYPE_SYSTEM;
            break;
        case FAT_ATTRIBUTE_SYSTEMDIR:
            file_type = FILE_TYPE_DIRECTORY|FILE_TYPE_SYSTEM;
            break;
        case FAT_ATTRIBUTE_NORMAL:
            file_type = FILE_TYPE_FILE;
            break;
        case FAT_ATTRIBUTE_DIRECTORY:
            file_type = FILE_TYPE_DIRECTORY;
            break;
    };
    dword cluster_location = (sfn_entry.starting_cluster_high << 16)|sfn_entry.starting_cluster_low;

    debug::out::printf("debugging the sfn entry...\n");
    debug::out::printf("sfn_entry.file_name         = \"%s\"\n" , sfn_entry.file_name);
    debug::out::printf("sfn_entry.attribute         = 0x%02x\n" , sfn_entry.attribute);
    debug::out::printf("sfn_entry.starting_cluster  = 0x%08x\n" , cluster_location);
    debug::out::printf("sfn_entry.file_size         = %dB\n" , sfn_entry.file_size);

    physical_file_location file_ploc = {
        .block_location = fat::cluster_to_sector(cluster_location , ginfo) , 
        .block_device = rootdir_loc->block_device , 
        .fs_driver = rootdir_loc->fs_driver ,  
    };
    return vfs::create_file_info_struct(file_ploc , file_name , file_type , sfn_entry.file_size);
}

static file_info *fat16_get_file_handle(const general_file_name file_name) {
    physical_file_location *rootdir_file_loc = fsdev::get_physical_loc_info(file_name.root_directory);
    // The juxtaposition of namespaces
    fat::general_fat_info ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(rootdir_file_loc->block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    fat16::get_ginfo(ginfo , &vbr);

    sfn_entry_t sfn_entry;
    if(fat::get_sfn_entry(rootdir_file_loc->block_device , rootdir_file_loc->block_location , file_name.file_name , &sfn_entry , ginfo) == false) {
        debug::out::printf("sfn entry not found!\n");
        return 0x00;
    }
    
    file_info *new_file = write_file_info_by_sfn(rootdir_file_loc , file_name.file_name , sfn_entry , ginfo);
    return new_file;
}

static bool fat16_remove(const general_file_name file_name) {
    return false;
}

static bool fat16_rename(const general_file_name file_name) {
    return false;
}

static bool fat16_move(const general_file_name file_name , file_info *new_directory) {
    return false;
}

static int fat16_read(file_info *file , size_t size , void *buffer) {
    return 0;
}

static int fat16_write(file_info *file , size_t size , const void *buffer) {
    return 0;
}

static int fat16_lseek(file_info *file , max_t cursor , int option) {
    return 0;
}

static bool fat16_get_root_directory(physical_file_location &file_loc) {
    struct fat::general_fat_info ginfo;
    fat16::fat16_vbr_t vbr;
    fat::get_vbr(file_loc.block_device , &vbr , sizeof(fat16::fat16_vbr_t));
    file_loc.block_location = fat16::get_root_directory(&vbr);
    return true;
}

static int fat16_read_directory(file_info *file , max_t cursor) {
    return 0x00;
}

void fat16::register_driver(void) {
    file_device_driver *drv = new file_device_driver;
    drv->check                  = fat16_check;
    drv->create                 = fat16_create;
    drv->get_file_handle        = fat16_get_file_handle;
    drv->remove                 = fat16_remove;
    drv->move                   = fat16_move;
    drv->read                   = fat16_read;
    drv->write                  = fat16_write;
    drv->lseek                  = fat16_lseek;
    drv->get_root_directory     = fat16_get_root_directory;
    drv->read_directory         = fat16_read_directory;
    
    fsdev::register_driver(drv , "fat16");
}