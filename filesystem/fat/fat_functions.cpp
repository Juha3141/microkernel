#include "fat.hpp"

#include <kernel/virtual_file_system.hpp>

file_info *fat::write_file_info_by_sfn(const physical_file_location *rootdir_loc , const char *file_name , sfn_entry_t &sfn_entry , fat::general_fat_info_t &ginfo) {
    int file_type = 0;
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
/*
    debug::out::printf("debugging the sfn entry...\n");
    debug::out::printf("sfn_entry.file_name         = \"%s\"\n" , sfn_entry.file_name);
    debug::out::printf("sfn_entry.attribute         = 0x%02x\n" , sfn_entry.attribute);
    debug::out::printf("sfn_entry.starting_cluster  = 0x%08x\n" , cluster_location);
    debug::out::printf("sfn_entry.file_size         = %dB\n" , sfn_entry.file_size);
*/
    
    physical_file_location file_ploc = {
        .block_location = fat::cluster_to_sector(cluster_location , ginfo) , 
        .block_device = rootdir_loc->block_device , 
        .fs_driver = rootdir_loc->fs_driver ,  
    };
    return vfs::create_file_info_struct(file_ploc , file_name , file_type , sfn_entry.file_size);
}

/*********************** Cluster Related ***********************/

void fat::get_vbr(blockdev::block_device *device , void *vbr , int vbr_sz) {
    int i;
    byte boot_sector[512];
    memset(boot_sector , 0 , 512);
    device->device_driver->read(device , 0 , 1 , boot_sector);

    memcpy(vbr , boot_sector , vbr_sz);
}

dword fat::cluster_to_sector(dword cluster_num , fat::general_fat_info_t &ginfo) { return ((cluster_num-2)*((common_vbr_t *)ginfo.vbr)->sectors_per_cluster)+ginfo.data_area_loc; }
dword fat::sector_to_cluster(dword sector_num , fat::general_fat_info_t &ginfo) { return ((sector_num-ginfo.data_area_loc)/((common_vbr_t *)ginfo.vbr)->sectors_per_cluster)+2; }

dword fat::read_cluster(blockdev::block_device *device , max_t cluster_number , max_t cluster_count , void *data , fat::general_fat_info_t &ginfo) {
    max_t i;
    max_t next_cluster_addr = cluster_number;
    common_vbr_t *c_vbr = (common_vbr_t *)ginfo.vbr;
    debug::out::printf("Reading Cluster, Cluster count to read : %d\n" , cluster_count);
    debug::out::printf("Cluster Number : %d\n" , cluster_number);
    debug::out::printf("Sector Address : %d\n" , cluster_to_sector(cluster_number , ginfo));
    for(i = 0; i < cluster_count; i++) {
        
        if(device->device_driver->read(device , cluster_to_sector(next_cluster_addr , ginfo) , c_vbr->sectors_per_cluster , (data+(i*c_vbr->sectors_per_cluster*c_vbr->bytes_per_sector))) != c_vbr->sectors_per_cluster*c_vbr->bytes_per_sector) break;
        next_cluster_addr = find_next_cluster(device , next_cluster_addr , ginfo);
        
        // not available?
        if(next_cluster_addr == ginfo.invalid_cluster_info) break;
    }
    return i;
}

dword fat::write_cluster(blockdev::block_device *device , max_t cluster_number , max_t cluster_count , void *data , fat::general_fat_info_t &ginfo) {
    max_t i;
    max_t next_cluster_addr = cluster_number;
    common_vbr_t *c_vbr = (common_vbr_t *)ginfo.vbr;
    for(i = 0; i < cluster_count; i++) {
        if(device->device_driver->write(device , cluster_to_sector(next_cluster_addr , ginfo) , c_vbr->sectors_per_cluster , (data+(i*c_vbr->sectors_per_cluster*c_vbr->bytes_per_sector))) != c_vbr->sectors_per_cluster*c_vbr->bytes_per_sector) {
            break;
        }
        next_cluster_addr = find_next_cluster(device , next_cluster_addr , ginfo);
        if(next_cluster_addr == ginfo.invalid_cluster_info) break;
    }
    return i;
}

static dword fat12_find_next_cluster(blockdev::block_device *device , dword cluster , fat::general_fat_info_t &ginfo) {
    max_t index = ((cluster/2)*3)+(cluster%2);
    max_t sector_address = ((max_t)(index/((common_vbr_t *)ginfo.vbr)->bytes_per_sector))+ginfo.fat_area_loc;
    byte fat_area[512];
    word ret_cluster;
    device->device_driver->read(device , sector_address , 1 , fat_area);
    max_t b_index = (max_t)(index%((common_vbr_t *)ginfo.vbr)->bytes_per_sector);
    
    ret_cluster = fat_area[b_index]|(fat_area[b_index+1] << 8);
    
    if(cluster%2) ret_cluster >>= 4; // odd
    else ret_cluster &= 0x0FFF;      // even
    
    return ret_cluster;
}

static dword fat16_find_next_cluster(blockdev::block_device *device , dword cluster , fat::general_fat_info_t &ginfo) {
    max_t cluster_info_per_sector = (((common_vbr_t *)ginfo.vbr)->bytes_per_sector/2);
    max_t sector_address = (max_t)(cluster/cluster_info_per_sector)+ginfo.fat_area_loc;
    byte fat_area[512];
    device->device_driver->read(device , sector_address , 1 , fat_area);
    return (fat_area[((cluster%cluster_info_per_sector)*2)])
          +(fat_area[((cluster%cluster_info_per_sector)*2)+1] << 8);
}

static dword fat32_find_next_cluster(blockdev::block_device *device , dword cluster , fat::general_fat_info_t &ginfo) {
    max_t cluster_info_per_sector = (((common_vbr_t *)ginfo.vbr)->bytes_per_sector/4);
    max_t sector_address = (max_t)(cluster/4)+ginfo.fat_area_loc;
    byte fat_area[512];
    device->device_driver->read(device , sector_address , 1 , fat_area);
    return (fat_area[((cluster%cluster_info_per_sector)*4)])
          +(fat_area[((cluster%cluster_info_per_sector)*4)+1] << 8)
          +(fat_area[((cluster%cluster_info_per_sector)*4)+2] << 16)
          +(fat_area[((cluster%cluster_info_per_sector)*4)+3] << 24);
}

dword fat::find_next_cluster(blockdev::block_device *device , dword cluster , fat::general_fat_info_t &ginfo) {
    switch(ginfo.fat_type) {
        case GINFO_FAT_TYPE_12:
            return fat12_find_next_cluster(device , cluster , ginfo);
        case GINFO_FAT_TYPE_16:
            return fat16_find_next_cluster(device , cluster , ginfo);
        case GINFO_FAT_TYPE_32:
            return fat32_find_next_cluster(device , cluster , ginfo);
    }
    return 0x00;
}

static max_t get_total_cluster_count(fat::general_fat_info_t &ginfo , max_t bytes_per_sector) {
    switch(ginfo.fat_type) {
        case GINFO_FAT_TYPE_12:
            return (max_t)((ginfo.fat_size*bytes_per_sector/2)*3);
        case GINFO_FAT_TYPE_16:
            return (max_t)(ginfo.fat_size*bytes_per_sector/2);
        case GINFO_FAT_TYPE_32:
            return (max_t)(ginfo.fat_size*bytes_per_sector/4);
    }
    return 0x00;
}

dword fat::find_first_empty_cluster(blockdev::block_device *device , fat::general_fat_info_t &ginfo) {
    common_vbr_t *c_vbr = (common_vbr_t *)ginfo.vbr;
    max_t total_cluster_count = get_total_cluster_count(ginfo , c_vbr->bytes_per_sector);
    for(int i = 3; i < total_cluster_count; i++) { // starting from Cluster #3
        if(find_next_cluster(device , i , ginfo) == 0x00) return i;
    }
    return 0xFFFFFFFF;
}

// Filter out the root directory before using it!
dword fat::get_file_cluster_count(blockdev::block_device *device , dword sector_number , fat::general_fat_info_t &ginfo) {
    max_t i;
    max_t next_cluster_addr;
    dword cluster_count = 0;

    // this is silly
    next_cluster_addr = sector_to_cluster(sector_number , ginfo);
    while(next_cluster_addr != 0x00) {
        next_cluster_addr = find_next_cluster(device , next_cluster_addr , ginfo);
        cluster_count++;
        if(next_cluster_addr == ginfo.invalid_cluster_info) break;
    }
    return cluster_count;
}

static void fat12_write_cluster_info(blockdev::block_device *device , dword cluster , word cluster_info , fat::general_fat_info_t &ginfo) {
    max_t index = ((cluster/2)*3)+(cluster%2);
    max_t sector_address = ((max_t)(index/((common_vbr_t *)ginfo.vbr)->bytes_per_sector))+ginfo.fat_area_loc;
    byte fat_area[512];
    device->device_driver->read(device , sector_address , 1 , fat_area);
    max_t b_index = (max_t)(index%((common_vbr_t *)ginfo.vbr)->bytes_per_sector);
    
    word cluster_data = fat_area[b_index]|(fat_area[b_index+1] << 8); 

    if(cluster%2) cluster_data = (cluster_info << 4)|(cluster_data & 0x0F);
    else cluster_data = (cluster_data & 0xF000)|(cluster_info & 0xFFF);

    fat_area[b_index] = cluster_data & 0xFF;
    fat_area[b_index+1] = (cluster_data >> 8) & 0xFF;

    device->device_driver->write(device , sector_address , 1 , fat_area);
    device->device_driver->write(device , sector_address+ginfo.fat_size , 1 , fat_area);
}

static void fat16_write_cluster_info(blockdev::block_device *device , dword cluster , word cluster_info , fat::general_fat_info_t &ginfo) {
    int cluster_info_per_sector = ((common_vbr_t *)ginfo.vbr)->bytes_per_sector/2;
    dword sector_addr = (dword)((cluster/cluster_info_per_sector)+ginfo.data_area_loc);
    common_vbr_t *c_vbr = (common_vbr_t *)ginfo.vbr;

    byte fat_area[c_vbr->bytes_per_sector];
    device->device_driver->read(device , sector_addr , 1 , fat_area);
    // possibly dangerous!
    memcpy(fat_area+((cluster%cluster_info_per_sector)*2) , &(cluster_info) , 2);

    device->device_driver->write(device , sector_addr , 1 , fat_area);
    device->device_driver->write(device , sector_addr+ginfo.fat_size , 1 , fat_area);
}

static void fat32_write_cluster_info(blockdev::block_device *device , dword cluster , word cluster_info , fat::general_fat_info_t &ginfo) {
    int cluster_info_per_sector = ((common_vbr_t *)ginfo.vbr)->bytes_per_sector/2;
    dword sector_addr = (dword)((cluster/cluster_info_per_sector)+ginfo.data_area_loc);
    common_vbr_t *c_vbr = (common_vbr_t *)ginfo.vbr;

    byte fat_area[c_vbr->bytes_per_sector];
    device->device_driver->read(device , sector_addr , 1 , fat_area);
    // possibly dangerous!
    memcpy(fat_area+((cluster%cluster_info_per_sector)*2) , &(cluster_info) , 2);

    device->device_driver->write(device , sector_addr , 1 , fat_area);
    device->device_driver->write(device , sector_addr+ginfo.fat_size , 1 , fat_area);
}

void fat::write_cluster_info(blockdev::block_device *device , dword cluster , max_t cluster_info , fat::general_fat_info_t &ginfo) {
    switch(ginfo.fat_type) {
        case GINFO_FAT_TYPE_12:
            return fat12_write_cluster_info(device , cluster , cluster_info , ginfo);
        case GINFO_FAT_TYPE_16:
            return fat16_write_cluster_info(device , cluster , cluster_info , ginfo);
        case GINFO_FAT_TYPE_32:
            return fat32_write_cluster_info(device , cluster , cluster_info , ginfo);
    }
    return;
}

/// @brief Add new clusters to the rear of a cluster
/// @param device Block device
/// @param end_cluster Location of target cluster
/// @param cluster_count Number of cluster to add
/// @param ginfo ginfo
void fat::extend_cluster(blockdev::block_device *device , dword end_cluster , dword cluster_count , fat::general_fat_info_t &ginfo) {
    int i;
    dword current_cluster = end_cluster;
    dword next_cluster;
    for(i = 0; i < cluster_count-1; i++) {
        next_cluster = find_first_empty_cluster(device , ginfo);
        write_cluster_info(device , current_cluster , next_cluster , ginfo);
        current_cluster = next_cluster;
    }
    write_cluster_info(device , current_cluster , ginfo.invalid_cluster_info , ginfo);
}

/*********************** Name Related ***********************/

int fat::get_filename_from_lfn(char *file_name , lfn_entry_t *entries) {
    int k = 0;
    word character;
    int lfn_entry_count = entries[0].seq_number^0x40; // Number of entry = First entry sequence number^0x40
    if(entries[0].seq_number == 0xE5) {
        return 0;
    }
    for(int i = lfn_entry_count-1; i >= 0; i--) {
        for(int j = 0; j < 5; j++) {
            file_name[k++] = (entries[i].file_name_1[j]) & 0xFF;
        }
        for(int j = 0; j < 6; j++) {
            file_name[k++] = (entries[i].file_name_2[j]) & 0xFF;
        }
        for(int j = 0; j < 2; j++) {
            file_name[k++] = (entries[i].file_name_3[j]) & 0xFF;
        }
    }
    file_name[k] = 0x00;
    return lfn_entry_count;
}

/// @brief Provide the basic information of a directory, specified by DirectorySectorAddress
/// @param device Targetted device
/// @param directory_sector_addr Sector address of the directory
/// @param dir_cluster_size (Return) the cluster size of directory
/// @return Number of entries that the directory has
dword fat::get_directory_info(blockdev::block_device *device , dword directory_sector_addr , fat::general_fat_info_t &ginfo) {
    int entry_count = 0;
    int cluster_count;
    int offset = 0;
    common_vbr_t *vbr;
    vbr = (common_vbr_t *)ginfo.vbr; // (this causes somehow modify lfn_entry[0] (in line 550~556).. I can't figure it why....)
    int next_cluster_address;
    byte *directory;
    sfn_entry_t *entry;
    if(directory_sector_addr == ginfo.root_dir_loc) {
        directory = (byte *)memory::pmem_alloc(ginfo.root_dir_size*vbr->bytes_per_sector);
        
        device->device_driver->read(device , directory_sector_addr , ginfo.root_dir_size , directory);
        while(1) {
            if(offset >= (ginfo.root_dir_size*vbr->bytes_per_sector)) break;
            entry = (sfn_entry_t *)((max_t)(directory+offset));

            if(entry->attribute == 0) break;
            entry_count++;
            offset += sizeof(sfn_entry_t);
        }
        memory::pmem_free(directory);
        return entry_count;
    }
    cluster_count = get_file_cluster_count(device , directory_sector_addr , ginfo);
    directory = (byte *)memory::pmem_alloc(cluster_count*vbr->sectors_per_cluster*vbr->bytes_per_sector);
    read_cluster(device , sector_to_cluster(directory_sector_addr , ginfo) , cluster_count , directory , ginfo);
    while(1) {
        entry = (sfn_entry_t *)((max_t)(directory+offset));
        if(entry->attribute == 0) break;

        entry_count++;
        offset += sizeof(sfn_entry_t);
    }
    memory::pmem_free(directory);
    return entry_count;
}

/// @brief Create 8.3 file name of original name
/// @param sfn_name Output 8.3 name
/// @param lfn_name original long file name
/// @param num Number of the name
void fat::create_sfn_name(char *sfn_name , const char *lfn_name , int num) {
    int i;
    int j;
    bool dot_exist = false;
    int dot_index = strlen(lfn_name);
    char buffer[strlen(lfn_name)+1];
    char number_str[32];
    char base_name[16];
    // Get index of dot
    if(strlen(lfn_name) == 0) return;
    if(strcmp(lfn_name , ".") == 0) { strcpy(sfn_name , ".          "); return; }
    if(strcmp(lfn_name , "..") == 0) { strcpy(sfn_name , "..         "); return; }
    memset(sfn_name , ' ' , 11);
    for(i = j = 0; lfn_name[i] != 0; i++) {
        if(lfn_name[i] != ' ') {
            buffer[j++] = lfn_name[i]; // Remove space
        }
    }
    buffer[j] = 0;
    for(i = 0; buffer[i] != 0; i++) {
        if(buffer[i] == '.') { // Get index of dot
            dot_index = i;
            dot_exist = true;
        }
        if((buffer[i] >= 'a') && (buffer[i] <= 'z')) { // Capitalize (Make a country capitalism)
            buffer[i] = (buffer[i]-'a')+'A';
        }
    }
    buffer[j] = 0x00;
    strncpy(base_name , buffer , ((dot_index <= 8) ? dot_index : 8));
    strcpy(sfn_name , base_name);
    for(i = 0; i < ((8-dot_index < 0) ? 0 : 8-dot_index); i++) {
        strcat(sfn_name , " ");
    }
    sprintf(number_str , "%d" , num);
    if(dot_index > 8) {
        // truncate
        sfn_name[strlen(sfn_name)-strlen(number_str)-1] = '~';
        strcpy(sfn_name+strlen(sfn_name)-strlen(number_str) , number_str);
    }
    if(dot_exist == false) strcat(sfn_name , "   ");
    else strncat(sfn_name , buffer+dot_index+1 , 3);
    // buffer : Capitalized, space removed file name
}

/// @brief Create SFN name for Volume Entry
/// @param sfn_name Output 8.3 name
/// @param lfn_name original long file name
void fat::create_volume_label_name(char *sfn_name , const char *lfn_name) {
    int i;
    int j;
    char buffer[strlen(lfn_name)+1];
    if(strlen(lfn_name) == 0) return;
    memset(sfn_name , ' ' , 11);
    for(i = j = 0; lfn_name[i] != 0; i++) {
        if(lfn_name[i] != ' ') {
            buffer[j++] = lfn_name[i]; // Remove space
        }
    }
    buffer[j] = 0x00;
    for(i = 0; buffer[i] != 0; i++) {
        if((buffer[i] >= 'a') && (buffer[i] <= 'z')) {
            buffer[i] = (buffer[i]-'a')+'A';
        }
    }
    memcpy(sfn_name , buffer , ((strlen(lfn_name) >= 11) ? 11 : strlen(lfn_name)));
}

byte fat::get_sfn_checksum(const char *sfn_name) {
    byte sum = 0;
    int check = 0;
    for(int i = 0; sfn_name[i] != 0; i++) {
        check = (check & 0x01) ? 0x80 : 0x00;
        sum = check+(sum >> 1);
        sum = sum+sfn_name[i];
        if(sum >= 0x100) {
            sum -= 0x100;
        }
        check = sum;
    }
    return sum;
}

bool fat::write_sfn_entry(blockdev::block_device *device , dword directory_addr , sfn_entry_t *entry , fat::general_fat_info_t &ginfo) {
    dword cluster_address;
    dword cluster_number;
    dword directory_cluster_size;
    int directory_entry_count;
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;

    byte cluster[vbr->sectors_per_cluster*vbr->bytes_per_sector];

// sector for root directory
    dword sector_addr = directory_addr;
    dword sector_number;
    dword directory_sector_size;

// to-do : Finish this!
    directory_cluster_size = get_file_cluster_count(device , directory_addr , ginfo);
    directory_entry_count = get_directory_info(device , directory_addr , ginfo);
    /*
    printf("DirectoryEntryCount : %d\n" , DirectoryEntryCount);
    // error
    printf("Location to write : %d\n" , directory_addr);
    */
    
    // If root directory, take care of it differently. 
    if(directory_addr == ginfo.root_dir_loc) {
        sector_number = (directory_entry_count*sizeof(sfn_entry_t))/vbr->bytes_per_sector;
        
        // error, device->Driver->ReadSector -> Invalid Opcode
        device->device_driver->read(device , sector_addr+sector_number , 1 , cluster);
        memcpy((cluster+((directory_entry_count*sizeof(sfn_entry_t))%vbr->bytes_per_sector)) , entry , sizeof(sfn_entry_t));
        device->device_driver->write(device , sector_addr+sector_number , 1 , cluster);
    }
    else {
        cluster_address = sector_to_cluster(directory_addr , ginfo);
        cluster_number = (directory_entry_count*sizeof(sfn_entry_t))/(vbr->bytes_per_sector*vbr->sectors_per_cluster);
        read_cluster(device , cluster_address+cluster_number , 1 , cluster , ginfo);
        
        memcpy(&(cluster[(directory_entry_count*sizeof(sfn_entry_t))%(vbr->bytes_per_sector*vbr->sectors_per_cluster)]) , 
        entry , sizeof(sfn_entry_t));
        write_cluster(device , cluster_address+cluster_number , 1 , cluster , ginfo);
    }
    
    return true;
}

bool fat::write_lfn_entry(blockdev::block_device *device , dword directory_addr , const char *file_name , general_fat_info_t &ginfo) {
    int i;
    int j;
    int name_offset = 0;
    byte checksum;
    int required_lfn_entry = (strlen(file_name)/13)+((strlen(file_name)%13 == 0) ? 0 : 1);
    lfn_entry_t lfn_entry[required_lfn_entry];
    char sfn_name[13];

// cluster for other directory
    byte *cluster;
    dword cluster_address;
    dword cluster_number;
    dword cluster_count;

    dword next_cluster;
    dword dir_cluster_size;
    int DirectoryEntryCount;
    common_vbr_t *vbr;

// sector for root directory
    dword sector_address = directory_addr;
    dword sector_number;
    dword sector_count;
    dword dir_sector_size;

    vbr = (common_vbr_t *)ginfo.vbr;
    create_sfn_name(sfn_name , file_name , 1); // To-do : number
    checksum = get_sfn_checksum(sfn_name);
    // Create LFN Entries
    /*
    printf("SFN file name   : %s\n" , sfn_name);
    printf("LFN Entry count : %d\n" , RequiredLFNEntry);
    */
    memset(lfn_entry , 0 , required_lfn_entry*sizeof(lfn_entry_t));
    for(i = required_lfn_entry-1; i >= 0; i--) {
        lfn_entry[i].attribute = 0x0F;
        lfn_entry[i].seq_number = (required_lfn_entry-i)|(((i == 0) ? 0x40 : 0));
        lfn_entry[i].checksum = checksum;
        lfn_entry[i].reserved = 0x00;
        lfn_entry[i].first_cluster_low = 0;
        for(j = 0; j < 5; j++) { lfn_entry[i].file_name_1[j] = ((name_offset <= strlen(file_name)) ? ((word)file_name[name_offset++]) : 0xFFFF); }
        for(j = 0; j < 6; j++) { lfn_entry[i].file_name_2[j] = ((name_offset <= strlen(file_name)) ? ((word)file_name[name_offset++]) : 0xFFFF); }
        for(j = 0; j < 2; j++) { lfn_entry[i].file_name_3[j] = ((name_offset <= strlen(file_name)) ? ((word)file_name[name_offset++]) : 0xFFFF); }
    }
    /*
    printf("directory Entry Count : %d\n" , DirectoryEntryCount);
    printf("lfn_entry              : 0x%X~0x%X\n" , lfn_entry , ((max_t)lfn_entry)+(RequiredLFNEntry*sizeof(lfn_entry_t)));
    printf("lfn_entry[0].Attribute : 0x%X\n" , lfn_entry[0].Attribute);
    printf("directory Address that's gonna be... : %d\n" , directory_addr);
    */
    dir_cluster_size = get_file_cluster_count(device , directory_addr , ginfo);
    DirectoryEntryCount = get_directory_info(device , directory_addr , ginfo);
    // If root directory, take care of it differently. 
    if(directory_addr) {
        sector_number = (DirectoryEntryCount*sizeof(sfn_entry_t))/vbr->bytes_per_sector;
        // Convert the offset of directory that's going to read - to sector number
        sector_count = ((required_lfn_entry*sizeof(sfn_entry_t))/vbr->bytes_per_sector)
                     +(((required_lfn_entry*sizeof(sfn_entry_t))%vbr->bytes_per_sector == 0) ? 0 : 1);
        // Convert the offset of Entry number to sector number
        cluster = (byte *)memory::pmem_alloc(vbr->bytes_per_sector*sector_count);
        device->device_driver->read(device , sector_address+sector_number , sector_count , cluster);

        memcpy(&(cluster[(DirectoryEntryCount*sizeof(sfn_entry_t))%(vbr->bytes_per_sector)]) , 
        lfn_entry , sizeof(sfn_entry_t)*required_lfn_entry);
        device->device_driver->write(device , sector_address+sector_number , sector_count , cluster);
    }
    else {
        cluster_address = sector_to_cluster(directory_addr , ginfo);
        cluster_number = (DirectoryEntryCount*sizeof(sfn_entry_t))/(vbr->bytes_per_sector*vbr->sectors_per_cluster);
        cluster_count = ((required_lfn_entry*sizeof(sfn_entry_t))/(vbr->bytes_per_sector*vbr->sectors_per_cluster))
                     +(((required_lfn_entry*sizeof(sfn_entry_t))%(vbr->bytes_per_sector*vbr->sectors_per_cluster) == 0) ? 0 : 1);
        cluster = (byte *)memory::pmem_alloc(vbr->sectors_per_cluster*vbr->bytes_per_sector*cluster_count);
        for(i = 0; i < cluster_number; i++) {
            next_cluster = find_next_cluster(device , cluster_address , ginfo);
            if(next_cluster == 0xFFFF) {
                break;
            }
            cluster_address = next_cluster;
        }
        read_cluster(device , cluster_address , cluster_count , cluster , ginfo);
        
        memcpy(&(cluster[(DirectoryEntryCount*sizeof(sfn_entry_t))%(vbr->bytes_per_sector*vbr->sectors_per_cluster)]) , 
        lfn_entry , sizeof(sfn_entry_t)*required_lfn_entry);
        write_cluster(device , cluster_address , cluster_count , cluster , ginfo);
    }
    return true;
}

bool fat::rewrite_sfn_entry(blockdev::block_device *device , dword directory_addr , const char *sfn_name , sfn_entry_t *new_sfn_entry , fat::general_fat_info_t &ginfo) {
    int i;
    int offset = 0;
    int cluster_number = 0;
    int entry_count = 0;
    dword dir_cluster_size;
    sfn_entry_t *sfn_entry;
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;
    byte *directory;
    entry_count = get_directory_info(device , directory_addr , ginfo);
    dir_cluster_size = get_file_cluster_count(device , directory_addr , ginfo);
    if(directory_addr == ginfo.root_dir_loc) {
        directory = (byte *)memory::pmem_alloc(ginfo.root_dir_size*vbr->bytes_per_sector);
        device->device_driver->read(device , directory_addr , ginfo.root_dir_size , directory);
        for(i = 0; i < ginfo.root_dir_size*vbr->bytes_per_sector/sizeof(sfn_entry_t); i++) {
            if(memcmp(((sfn_entry_t *)(directory+(offset)))->file_name , sfn_name , 11) == 0) {
                memcpy((sfn_entry_t *)(directory+(offset)) , new_sfn_entry , sizeof(sfn_entry_t));
                device->device_driver->write(device , directory_addr , ginfo.root_dir_size , directory);
                memory::pmem_free(directory);
                return true;
            }
            offset += sizeof(sfn_entry_t);
        }
        memory::pmem_free(directory);
        return false;
    }
    cluster_number = sector_to_cluster(directory_addr , ginfo);
    for(i = 0; i < dir_cluster_size; i++) {
        directory = (byte *)memory::pmem_alloc(vbr->sectors_per_cluster*vbr->bytes_per_sector);
        read_cluster(device , cluster_number , 1 , directory , ginfo);
        for(offset = 0; offset < vbr->sectors_per_cluster*vbr->bytes_per_sector; offset += sizeof(sfn_entry_t)) {
            if(((sfn_entry_t *)(directory+(offset)))->attribute == 0) {
                memory::pmem_free(directory);
                return false;
            }
            if(memcmp(((sfn_entry_t *)(directory+(offset)))->file_name , sfn_name , 11) == 0) {
                memcpy((sfn_entry_t *)(directory+(offset)) , new_sfn_entry , sizeof(sfn_entry_t));
                write_cluster(device , cluster_number , 1 , directory , ginfo);
                memory::pmem_free(directory);
                return true;
            }
        }
        cluster_number = find_next_cluster(device , cluster_number , ginfo);
    }
    memory::pmem_free(directory);
    return false;
}

/// @brief Mark entire entry(including LFN entry) removed
/// @param device Target storage
/// @param directory_addr Sector address of the directory
/// @param offset Absolute offset from the start of the directory
static void remove_entry_by_sfn_offset(blockdev::block_device *device , dword directory_addr , int offset , fat::general_fat_info_t &ginfo) {
    int sequence_number;
    int previous_cluster_number;
    int cluster_number;
    int lfn_start_offset;
    int cluster_offset; // offset in cluster number
    int relative_offset; // offset corresponds to directory
    byte *directory;
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;
    cluster_offset = offset/(vbr->sectors_per_cluster*vbr->bytes_per_sector);
    cluster_number = sector_to_cluster(directory_addr , ginfo);
    for(int i = 0; i < cluster_offset; i++) {
        previous_cluster_number = cluster_number;
        cluster_number = find_next_cluster(device , cluster_number , ginfo);
    }
    directory = (byte *)memory::pmem_alloc(vbr->sectors_per_cluster*vbr->bytes_per_sector*3);
    if(cluster_number != sector_to_cluster(directory_addr , ginfo)) {
        // The entry could intersect with two cluster; We should read two cluster in order to read one entry completely
        read_cluster(device , previous_cluster_number , 2 , directory , ginfo);
        relative_offset = (offset%(vbr->sectors_per_cluster*vbr->bytes_per_sector))+(vbr->sectors_per_cluster*vbr->bytes_per_sector);
    }
    else {
        // Not intersected with two cluster
        read_cluster(device , cluster_number , 1 , directory , ginfo);
        relative_offset = offset%(vbr->sectors_per_cluster*vbr->bytes_per_sector);
    }
    if(((sfn_entry_t *)(directory+relative_offset-sizeof(sfn_entry_t)))->attribute != FAT_ATTRIBUTE_LFN) {
        // Current entry : Offsets
        ((sfn_entry_t *)(directory+relative_offset))->file_name[0] = FAT_FILENAME_REMOVED;
        memory::pmem_free(directory);
        return;
    }

    if((((lfn_entry_t *)(directory+relative_offset-sizeof(lfn_entry_t)))->seq_number & 0x40) == 0x40) {
        sequence_number = ((lfn_entry_t *)(directory+relative_offset-sizeof(lfn_entry_t)))->seq_number^0x40;
    }
    // There is more than one entry there
    else {
        sequence_number = ((lfn_entry_t *)(directory+relative_offset-sizeof(lfn_entry_t)))->seq_number+1;
    }
    // Determine Start offset of the LFN
    lfn_start_offset = relative_offset-sequence_number*sizeof(sfn_entry_t);
    // Mark every LFN entry as removed
    for(int j = relative_offset; j >= lfn_start_offset; j -= sizeof(lfn_entry_t)) {
        // In FAT16, when first character of file name is 0xE5, it is considered as removed file.
        ((sfn_entry_t *)(directory+j))->file_name[0] = FAT_FILENAME_REMOVED;
    }
    // Write the modified version of directory
    if(cluster_number != sector_to_cluster(directory_addr , ginfo)) {
        write_cluster(device , previous_cluster_number , 2 , directory , ginfo);
    }
    else {
        write_cluster(device , cluster_number , 1 , directory , ginfo);
    }
    memory::pmem_free(directory);
    return;
}

bool fat::mark_entry_removed(blockdev::block_device *device , dword directory_addr , const char *sfn_name , general_fat_info_t &ginfo) {
    int i;
    int j;
    int lfn_start_offset;
    int seq_number;
    int offset = 0;
    int absolute_offset = 0;
    int cluster_number = 0;
    int entry_count = 0;
    unsigned int directory_cluster_size;
    sfn_entry_t *sfn_entry;
    lfn_entry_t *lfn_entry;
    byte *directory;
    /*
     * To-do : Add comment & whatever
    */
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;
    // Get information of directory
    entry_count = get_directory_info(device , directory_addr , ginfo);
    directory_cluster_size = get_file_cluster_count(device , directory_addr , ginfo);
    // Process it differently when it's a root directory
    if(directory_addr == ginfo.root_dir_loc) {
        // Root directory is small; we can just get entire root directory and process it(straightforward!)
        directory = (byte *)memory::pmem_alloc(ginfo.root_dir_loc*vbr->bytes_per_sector);
        device->device_driver->read(device , directory_addr , ginfo.root_dir_loc , directory);

        // printf("sfn_name : %s\n" , sfn_name);
        for(i = 0; i < ginfo.root_dir_loc*vbr->bytes_per_sector/sizeof(sfn_entry_t); i++) {
            // Search file each
            if(memcmp(((sfn_entry_t *)(directory+offset))->file_name , sfn_name , 11) == 0) {
                // If it's not a LFN entry file that behinds current entry, only current entry should be removed
                if(((sfn_entry_t *)(directory+offset-sizeof(sfn_entry_t)))->attribute != FAT_ATTRIBUTE_LFN) {
                    // Current entry : Offsets
                    ((sfn_entry_t *)(directory+offset))->file_name[0] = FAT_FILENAME_REMOVED;
                    device->device_driver->write(device , directory_addr , ginfo.root_dir_loc , directory);
                    memory::pmem_free(directory);
                    return true;
                }
                // Get the sequence number from previous entry
                // There is only one LFN entry there
                if((((lfn_entry_t *)(directory+offset-sizeof(lfn_entry_t)))->seq_number & 0x40) == 0x40) {
                    seq_number = ((lfn_entry_t *)(directory+offset-sizeof(lfn_entry_t)))->seq_number^0x40;
                }
                // There is more than one entry there
                else {
                    seq_number = ((lfn_entry_t *)(directory+offset-sizeof(lfn_entry_t)))->seq_number+1;
                }
                // Determine Start offset of the LFN
                lfn_start_offset = offset-seq_number*sizeof(sfn_entry_t);
                // Mark every LFN entry as removed
                for(j = offset; j >= lfn_start_offset; j -= sizeof(lfn_entry_t)) {
                    // In FAT16, when first character of file name is 0xE5, it is considered as removed file.
                    ((sfn_entry_t *)(directory+j))->file_name[0] = FAT_FILENAME_REMOVED;
                }
                // Write the modified version of directory
                device->device_driver->write(device , directory_addr , ginfo.root_dir_size , directory);
                memory::pmem_free(directory);
                return true;
            }
            // Keep searching
            offset += sizeof(sfn_entry_t);
        }
        // Failed to search
        memory::pmem_free(directory);
        return false;
    }
    // Not a root directory, should be process differently by just partially reading the contents of the directory
    cluster_number = sector_to_cluster(directory_addr , ginfo);
    for(i = 0; i < directory_cluster_size; i++) {
        directory = (byte *)memory::pmem_alloc(vbr->sectors_per_cluster*vbr->bytes_per_sector);
        read_cluster(device , cluster_number , 1 , directory , ginfo);
        for(offset = 0; offset < vbr->sectors_per_cluster*vbr->bytes_per_sector; offset += sizeof(sfn_entry_t)) {
            if(((sfn_entry_t *)(directory+(offset)))->attribute == 0) {
                memory::pmem_free(directory);
                return false;
            }
            // Found the file by the SFN name
            if(memcmp(((sfn_entry_t *)(directory+(offset)))->file_name , sfn_name , 11) == 0) {
                // Absolute offset : Based on the "Start" of the directory
                absolute_offset = offset+(i*vbr->sectors_per_cluster*vbr->bytes_per_sector);
                debug::out::printf("Found file!!, Absolute offset : %d\n" , offset);
                remove_entry_by_sfn_offset(device , directory_addr , offset , ginfo);
                memory::pmem_free(directory);
                return true;
            }
        }
        cluster_number = find_next_cluster(device , cluster_number , ginfo);
    }
    memory::pmem_free(directory);
    return false;
}

bool fat::get_sfn_entry(blockdev::block_device *device , dword directory_addr , const char *file_name , sfn_entry_t *destination , general_fat_info_t &ginfo) {
    int i;
    int offset = 0;
    int entry_count;
    int lfn_entry_count;
    dword dir_cluster_size;
    sfn_entry_t *sfn_entry;
    lfn_entry_t *lfn_entry;
    byte *directory;
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;

    entry_count = get_directory_info(device , directory_addr , ginfo);
    dir_cluster_size = get_file_cluster_count(device , directory_addr , ginfo);
    directory = (byte *)memory::pmem_alloc(dir_cluster_size*vbr->sectors_per_cluster*vbr->bytes_per_sector);
    debug::out::printf("entry_count   : %d\n" , entry_count);
    debug::out::printf("cluster count : %d\n" , dir_cluster_size);
    if(directory_addr == ginfo.root_dir_loc) {
        device->device_driver->read(device , directory_addr , ginfo.root_dir_size , directory);
    }
    else {
        read_cluster(device , sector_to_cluster(directory_addr , ginfo) , dir_cluster_size , directory , ginfo);
    }
    debug::out::printf("dumping directory... size : %d\n" , vbr->bytes_per_sector);
    char temp_file_name[(entry_count*(5+6+2))+1];
    create_sfn_name(temp_file_name , file_name , 1);
    /*
    if((strlen(FileName) <= 11)) { // bug
        // printf("TemporaryFileName : \"%s\"(%d char)\n" , TemporaryFileName , strlen(TemporaryFileName));
        for(i = 0; i < entry_count; i++) {
            sfn_entry = (sfn_entry_t *)(directory+offset);
            if(memcmp(TemporaryFileName , sfn_entry->FileName , 11) == 0) {
                memcpy(Destination , sfn_entry , sizeof(sfn_entry_t));
                memory::pmem_free(directory);
                return true;
            }
            offset += sizeof(sfn_entry_t);
        }
        memory::pmem_free(directory);
        return false;
    }
    */
    debug::out::printf("directory location : %d\n" , directory_addr);
    for(i = 0; i < entry_count; i++) {
        sfn_entry = (sfn_entry_t *)(directory+offset);
        lfn_entry = (lfn_entry_t *)(directory+offset);
        if(lfn_entry->attribute == FAT_ATTRIBUTE_LFN) {
            lfn_entry_count = lfn_entry->seq_number^0x40;
            if(get_filename_from_lfn(temp_file_name , lfn_entry) == 0) {
                offset += sizeof(sfn_entry_t);
                continue;
            }
            debug::out::printf_function(DEBUG_INFO , "fat::get_sfn_entr" , "file_name : %s , temp_file_name : %s\n" , file_name , temp_file_name);
            if(strcmp(file_name , temp_file_name) == 0) {
                offset += lfn_entry_count*sizeof(lfn_entry_t);
                memcpy(destination , (sfn_entry_t *)(directory+offset) , sizeof(sfn_entry_t));
                memory::pmem_free(directory);
                return true;
            }
            memset(temp_file_name , 0 , strlen(temp_file_name));
            offset += sizeof(lfn_entry_t)*lfn_entry_count;
        }
        else {
            offset += sizeof(sfn_entry_t);
        }
    }
    memory::pmem_free(directory);
    return false;
}

int fat::get_file_list(physical_file_location *dir_location , ObjectLinkedList<file_info> &file_list , general_fat_info_t &ginfo) {
    int i;
    int offset = 0;
    int entry_count;
    int lfn_entry_count;
    dword dir_cluster_size;
    sfn_entry_t *sfn_entry;
    lfn_entry_t *lfn_entry;
    byte *directory;
    common_vbr_t *vbr = (common_vbr_t *)ginfo.vbr;

    entry_count = get_directory_info(dir_location->block_device , dir_location->block_location , ginfo);
    dir_cluster_size = get_file_cluster_count(dir_location->block_device , dir_location->block_location , ginfo);
    directory = (byte *)memory::pmem_alloc(dir_cluster_size*vbr->sectors_per_cluster*vbr->bytes_per_sector);
    debug::out::printf("entry_count   : %d\n" , entry_count);
    debug::out::printf("cluster count : %d\n" , dir_cluster_size);
    if(dir_location->block_location == ginfo.root_dir_loc) {
        dir_location->block_device->device_driver->read(dir_location->block_device , dir_location->block_location , ginfo.root_dir_size , directory);
    }
    else {
        read_cluster(dir_location->block_device , sector_to_cluster(dir_location->block_location , ginfo) , dir_cluster_size , directory , ginfo);
    }
    
    int file_count = 0;
    char temp_file_name[(entry_count*(5+6+2))+1];
    debug::out::printf("directory location : %d\n" , dir_location->block_location);
    for(i = 0; i < entry_count; i++) {
        sfn_entry = (sfn_entry_t *)(directory+offset);
        lfn_entry = (lfn_entry_t *)(directory+offset);
        if(lfn_entry->attribute == FAT_ATTRIBUTE_LFN) {
            lfn_entry_count = lfn_entry->seq_number^0x40;
            if(get_filename_from_lfn(temp_file_name , lfn_entry) == 0) {
                offset += sizeof(sfn_entry_t);
                continue;
            }
            
            // increment the offset
            offset += lfn_entry_count*sizeof(lfn_entry_t);
            file_count++;

            sfn_entry_t entry;
            // copy the entry
            memcpy(&entry , (sfn_entry_t *)(directory+offset) , sizeof(sfn_entry_t));
            file_info *new_file_info = write_file_info_by_sfn(dir_location , temp_file_name , entry , ginfo);
            file_list.add_object_rear(new_file_info);
        }
        else offset += sizeof(sfn_entry_t);
    }
    memory::pmem_free(directory);
    return file_count;
}