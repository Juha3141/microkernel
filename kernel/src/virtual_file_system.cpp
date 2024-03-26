#include <kernel/virtual_file_system.hpp>
#include <kernel/file_system_driver.hpp>

#include <kernel/debug.hpp>

////// VirtualFileSystemManager

void vfs::VirtualFileSystemManager::init(file_info *rdir , char dir_ident)  {
    this->fs_root_dir = rdir;
    fs_root_dir->parent_dir = 0x00;
    fs_root_dir->file_lists = 0x00;
    dir_identifier = dir_ident;
    is_initialized_properly = true;
}

void vfs::VirtualFileSystemManager::add_object(file_info *file , file_info *directory) {
    if(directory->file_lists == 0x00) {
        directory->file_lists = new ObjectLinkedList<file_info_s>;
        directory->file_lists->init();
    }
    directory->file_lists->add_object_rear(file);
    file->parent_dir = directory;
}

bool vfs::VirtualFileSystemManager::remove_object(file_info *file) {
    return false;
}

file_info *vfs::VirtualFileSystemManager::search_object_last(int level_count , file_info *search_root , char **file_links , int &last_hit_loc) {
    file_info *ptr = search_root;
    int i = 0;
    // If file links start with root directory
    if(strcmp(file_links[0] , fs_root_dir->file_name) == 0) {
        // root path is root directory
        ptr = fs_root_dir;
        i++;
    }
    for(; i < level_count; i++) {
        last_hit_loc = i;
        if(ptr->file_lists == 0x00) return ptr;
        ObjectLinkedList<file_info_s>::node_s *node = ptr->file_lists->search<char*>([](file_info_s *obj , char *str) { return (strcmp(obj->file_name , str) == 0); } , file_links[i]);
        if(node == 0x00) return ptr;

        ptr = node->object;
    }
    return ptr;
}

/// @brief Get how much directory the file is referencing from the file name
/// @param file_name file name
/// @return number of directories
int vfs::VirtualFileSystemManager::auto_parse_dir_count(const char *file_name) {
    int dir_count = 1;
    for(int i = 0; file_name[i] != 0; i++) {
        if(file_name[i] == dir_identifier) {
            dir_count++;
        }
    }
    return dir_count;
}

/// @brief Parse the file name into lists of directories
/// @param file_name file name
/// @param parsed parsed list
/// @return number of items in the list
int vfs::VirtualFileSystemManager::auto_parse_name(const char *file_name , char **parsed) {    // Auto-allocates
    int i = 0;
    int j = 0;
    int prev_index = 0;
    int dir_count = 0;
    if((dir_count = auto_parse_dir_count(file_name)) != 0) {
        for(i = 0; file_name[i] != 0; i++) {
            if(file_name[i] == dir_identifier) {
                parsed[j] = (char *)memory::pmem_alloc(i-prev_index+1);
                strncpy(parsed[j] , file_name+prev_index , i-prev_index);
                prev_index = i+1;
                j++;
            }
        }
    }
    parsed[j] = (char *)memory::pmem_alloc(i-prev_index+1);
    strncpy(parsed[j] , file_name+prev_index , i-prev_index);
    return dir_count;
}

/// Standard vfs functions

void vfs::init(blockdev::block_device *root_device) {
    debug::push_function("vfs::init");
    file_info *root_file = (file_info *)memory::pmem_alloc(sizeof(file_info));
    strcpy(root_file->file_name , "@");
    root_file->is_mounted = true;

    // mount the file
    if(vfs::mount(root_file , root_device) == false) {
        debug::out::printf(DEBUG_WARNING , "Failed mounting root device!\n");
        GLOBAL_OBJECT(VirtualFileSystemManager)->is_initialized_properly = false;
        return;
    }
    debug::out::printf("");
    debug::out::printf(DEBUG_SPECIAL , "fs_driver : 0x%lx\n" , root_file->mount_loc_info.fs_driver);
    debug::out::printf(DEBUG_SPECIAL , "Device %s%d : File system detected, %s\n" , root_device->device_driver->driver_name , root_device->id , root_file->mount_loc_info.fs_driver->fs_string);
    GLOBAL_OBJECT(VirtualFileSystemManager)->init(root_file , '/');
    debug::pop_function();
}

file_info *vfs::get_root_directory(void) { return GLOBAL_OBJECT(VirtualFileSystemManager)->fs_root_dir; }

file_info *vfs::create_file_info_struct(
        const physical_file_location file_loc ,
        const char *file_name ,
        int file_type ,
        int file_size) {
    file_info *new_file = new file_info;
    memset(new_file , 0 , sizeof(file_info));
    // file name
    strcpy(new_file->file_name , file_name);
    // mount flag
    new_file->is_mounted = false;
    // file physical location
    memcpy(&new_file->file_loc_info , &file_loc , sizeof(physical_file_location));
    // file type
    new_file->file_type = file_type;

    new_file->file_size = file_size;
    new_file->who_open_list = new ObjectLinkedList<open_info_t>;
    new_file->who_open_list->init();
    new_file->disk_buffer_hash_table = new HashTable<block_cache_t , max_t>;
    new_file->disk_buffer_hash_table->init(512 , [](max_t&d,max_t s){d=s;} , [](max_t d,max_t s){ return (bool)(d==s); } , 
    [](max_t key) { return (hash_index_t)(key%512); });
    
    return new_file;
}

bool vfs::mount(file_info *file , blockdev::block_device *device) {
    fsdev::file_system_driver *fs_driver = fsdev::detect_fs(device);
    if(fs_driver == 0x00) return false;
    file->mount_loc_info.fs_driver = fs_driver;
    file->mount_loc_info.block_device = device;
    if(fs_driver->get_root_directory(file->mount_loc_info) == false) return false; 

    file->is_mounted = true;
    return true;
}

static void get_highest_level_file_name(const char *original_name , char *output) {
    int j = 0;
    int len = strlen(original_name);
    int last_loc = len-1;
    char identifier = GLOBAL_OBJECT(vfs::VirtualFileSystemManager)->dir_identifier;
    char *str;
    for(; last_loc >= 0; last_loc--) {
        if(original_name[last_loc] == identifier) break;
    }
    last_loc++;
    
    memcpy(output , original_name+last_loc , len-last_loc);
    output[len-last_loc] = 0x00;
}

/// @brief Automatically make a space for file names, parse the file name and store it to the allocated space
///        Important! Please discard the memory after using the file list
/// @param file_path File path to be parsed
/// @param file_list the resulting file list
/// @return Number of parsed names
static int get_file_name_list(const general_file_name file_path , char **(&file_list)) {
    vfs::VirtualFileSystemManager *vfs_mgr = GLOBAL_OBJECT(vfs::VirtualFileSystemManager);
    int lvl_count = vfs_mgr->auto_parse_dir_count(file_path.file_name);
    file_list = (char **)memory::pmem_alloc(lvl_count*sizeof(char*));
    vfs_mgr->auto_parse_name(file_path.file_name , file_list);
    return lvl_count;
}

/// @brief Get the file_info handle by using cache. 
///        If there is no file on the cache tree, function physically searches for the file.
///        The files opened for directory searching is stored into cache tree. 
/// @param file_path General file path
/// @param levels_to_exclude Number of directory levels to exclude from searching
/// @return file_info handle, if failed searching(file does not exist), return 0x00
static file_info *get_file_by_cache_and_phys(const general_file_name file_path , int levels_to_exclude) {
    vfs::VirtualFileSystemManager *vfs_mgr = GLOBAL_OBJECT(vfs::VirtualFileSystemManager);
    char **file_list;
    int level_count;
    int last_hit_loc = 0;

    // failed to initialize the vfs manager
    if(!vfs_mgr->is_initialized_properly) return 0x00;

    level_count = get_file_name_list(file_path , file_list);

    file_info *file = vfs_mgr->search_object_last(level_count , file_path.root_directory , file_list , last_hit_loc);
    if(file != 0x00 && strcmp(file->file_name , file_list[level_count-1]) == 0) { // cash hit
        memory::pmem_free(file_list);

        return file;
    }
    debug::out::printf_function(DEBUG_TEXT , "get_file_by_cache" , "last cash hit : %s(file=0x%lx), hit_loc : %d\n" , file->file_name , file , last_hit_loc);
    
    file_info *tree_file = file;
    for(int i = last_hit_loc; i < level_count-levels_to_exclude; i++) {
        physical_file_location *pfileloc = &tree_file->file_loc_info;
        if(tree_file->is_mounted == true) pfileloc = &tree_file->mount_loc_info;

        file_info *new_file_handle = pfileloc->fs_driver->get_file_handle({file_list[i] , tree_file});
        if(new_file_handle == 0x00) {
            memory::pmem_free(file_list);
            return 0x00;
        }

        vfs_mgr->add_object(new_file_handle , tree_file);
        tree_file = new_file_handle;
    }

    memory::pmem_free(file_list);
    return tree_file;
}

bool vfs::create(const general_file_name file_path , word file_type) {
    file_info *directory;
    physical_file_location *physical_loc;
    char top_name[strlen(file_path.file_name)];

    directory = get_file_by_cache_and_phys(file_path , 1);
    if(directory == 0x00) return false;

    physical_loc = fsdev::get_physical_loc_info(directory);
    get_highest_level_file_name(file_path.file_name , top_name);
    debug::out::printf("top_name : %s\n" , top_name);

    if(physical_loc->fs_driver == 0x00) return false;
    return physical_loc->fs_driver->create({top_name , directory} , file_type);
}

file_info *vfs::open(const general_file_name file_path , int option) {
    open_info_t *open_info = (open_info_t *)memory::pmem_alloc(sizeof(open_info_t));
    file_info *file = get_file_by_cache_and_phys(file_path , 0);
    max_t current_task_id = 0x00; // currently not implemented yet!
    if(file == 0x00) return 0x00;

    open_info->maximum_offset = file->file_size;
    open_info->open_offset = 0;
    open_info->open_flag = option;
    open_info->task_id = current_task_id;
    debug::out::printf("who_open_list : 0x%lx\n" , file->who_open_list);
    file->who_open_list->add_object_rear(open_info);
    return file;
}

bool vfs::close(file_info *file) {
    return 0x00;
}

bool vfs::remove(const general_file_name file_path) {
    return 0x00;
}


bool vfs::rename(const general_file_name file_path) {
    return 0x00;
}

bool vfs::move(const general_file_name file_path , const general_file_name new_directory) {
    return 0x00;
}

/// @brief Get the cache object from buffer cache. If the cache in block does not exist 
/// @param file File handle
/// @param linear_block_addr Linear block address
/// @return block cache object
static block_cache_t *get_buffer_by_cache_and_phys(file_info *file , max_t linear_block_addr) {
    block_cache_t *cache;

    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    max_t block_size = file_loc->block_device->geometry.block_size;
    max_t phys_block_loc = file_loc->fs_driver->get_phys_block_address(file , linear_block_addr);
    if(phys_block_loc == INVALID) { // does not exist, need to create new one
        return 0x00;
    }

    // cache does not exist, create new cache
    debug::out::printf("phys_block_loc : %d\n" , phys_block_loc);
    if((cache = file->disk_buffer_hash_table->search(phys_block_loc)) == 0x00) {
        cache = (block_cache_t *)memory::pmem_alloc(sizeof(block_cache_t));
        cache->block = (void *)memory::pmem_alloc(block_size);
        cache->block_size = block_size;
        file->disk_buffer_hash_table->add(phys_block_loc , cache);
        
        // block device as a unit
        file_loc->block_device->device_driver->read(file_loc->block_device , phys_block_loc , 1 , cache->block);
    }
    return cache;
}

long vfs::read(file_info *file , max_t size , void *buffer) {
    debug::push_function("vfs::read");
    max_t current_task_id;

    max_t open_offset = 0;
    max_t end_offset = 0;
    max_t block_size;
    max_t block_start;
    max_t block_end;
    max_t block_count;
    
    physical_file_location *file_loc;
    if(file->who_open_list == 0x00) return 0; // error
    current_task_id = 0x00; // currently not implemented yet!
    
    ObjectLinkedList<open_info_t>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
    if(who_opened == 0x00) return 0; // If not opened -- 

    file_loc = fsdev::get_physical_loc_info(file);

    // fill out basic informations
    block_size = file_loc->block_device->geometry.block_size;
    open_offset = who_opened->object->open_offset;
    end_offset = MIN(who_opened->object->maximum_offset , open_offset+size);
    block_start = open_offset/block_size;
    block_end = (open_offset+size)/block_size;
    block_count = block_end-block_start+1;
/*
    debug::out::printf("block_size : %d\n" , block_size);

    debug::out::printf("read size : %d\n" , end_offset-open_offset);

    debug::out::printf("open_offset : %d\n" , open_offset);
    debug::out::printf("block_start : %ld\n" , block_start);
    debug::out::printf("block_end   : %ld\n" , block_end);
*/
    block_cache_t *caches[block_count];

    // Calculate & Copy
    max_t off = open_offset;
    max_t buffer_offset = 0; 
    max_t read_size = 0;
    for(max_t b = block_start; b <= block_end; b++) {
        caches[b-block_start] = get_buffer_by_cache_and_phys(file , b);
        debug::out::printf("cache : 0x%X\n" , caches[b-block_start]);
        max_t boff = off%block_size;
        max_t bsize = MIN(end_offset-off , block_size-boff);
        memcpy(buffer+buffer_offset , caches[b-block_start]->block+boff , bsize);
        buffer_offset += bsize;
        off += bsize;
        read_size += bsize;
    }
    who_opened->object->open_offset += read_size;
    debug::pop_function();
    debug::out::printf("read_size : %d\n" , read_size);
    return read_size;
}

long vfs::write(file_info *file , max_t size , const void *buffer) {
    debug::push_function("vfs::write");
    max_t current_task_id;

    max_t open_offset = 0;
    max_t block_size;
    max_t create_size = 0;

    max_t block_start;
    max_t block_end;
    max_t block_count;

    physical_file_location *file_loc;
    if(file->who_open_list == 0x00) return 0; // error
    current_task_id = 0x00; // currently not implemented yet!
    
    ObjectLinkedList<open_info_t>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
    if(who_opened == 0x00) return 0;

    file_loc = fsdev::get_physical_loc_info(file);

    // fill out basic informations
    block_size = file_loc->block_device->geometry.block_size;
    open_offset = who_opened->object->open_offset;
    create_size = (open_offset+size > who_opened->object->maximum_offset) ? open_offset+size-who_opened->object->maximum_offset : 0;
    block_start = open_offset/block_size;
    block_end = (open_offset+size)/block_size;
    block_count = block_end-block_start+1;

    debug::out::printf("file_size   = %d\n" , who_opened->object->maximum_offset);
    debug::out::printf("open_offset = %d\n" , open_offset);
    debug::out::printf("size        = %d\n" , size);
    debug::out::printf("create_size = %d\n" , create_size);

    max_t required_block_count = (create_size/block_size)+(create_size%block_size != 0);
    if(create_size != 0) {
        max_t block_created = 0;
        debug::out::printf("required_bs = %d\n" , required_block_count);
        while(block_created < required_block_count) {
            max_t block_loc , unit_size = 1;
            block_loc = file_loc->fs_driver->allocate_new_block(file , unit_size);
            block_created += unit_size;
            debug::out::printf("new_block_location : %d\n" , block_loc);
        }
    }
    // Calculate & Copy

    max_t off = open_offset;
    max_t buffer_offset = 0; 
    max_t write_size = 0;
    block_cache_t *caches[block_count];
    debug::out::printf("block_start : %d\n" , block_start);
    debug::out::printf("block_end   : %d\n" , block_end);
    for(max_t b = block_start; b <= block_end; b++) {
        caches[b-block_start] = get_buffer_by_cache_and_phys(file , b);
        max_t boff = off%block_size;
        max_t bsize = MIN((open_offset+size)-off , block_size-boff);
        debug::out::printf("cache : 0x%X\n" , caches[b-block_start]);
        debug::out::printf("buffer_offset = %d\n" , buffer_offset);
        debug::out::printf("block_start   = %d\n" , block_start);
        memcpy(caches[b-block_start]->block+boff , buffer+buffer_offset , bsize);

        buffer_offset += bsize;
        off += bsize;
        write_size += bsize;
    }
    debug::out::printf("write_size : %d\n" , write_size);
    who_opened->object->open_offset += write_size;
    who_opened->object->maximum_offset = MAX(who_opened->object->open_offset , who_opened->object->maximum_offset);
    debug::pop_function();
    return write_size;
}


long vfs::lseek(file_info *file , long cursor , int option) {
    max_t current_task_id;
    current_task_id = 0x00; /* Not implemented */

    ObjectLinkedList<open_info_t>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
    if(who_opened == 0x00) return -1;
    debug::out::printf_function(DEBUG_TEXT , "vfs::lseek" , "(before) open_offset = %d\n" , who_opened->object->open_offset);
    switch(option) {
        case LSEEK_SET:
            if(cursor < 0) { who_opened->object->open_offset = 0; break; }
            who_opened->object->open_offset = cursor;
            break;
        case LSEEK_END:
            who_opened->object->open_offset = who_opened->object->maximum_offset;
            break;
        case LSEEK_CUR:
            if(who_opened->object->open_offset < -cursor) { who_opened->object->open_offset = 0; break; }
            who_opened->object->open_offset += cursor;
            break;
    }
    who_opened->object->open_offset = MIN(who_opened->object->open_offset , who_opened->object->maximum_offset);
    debug::out::printf_function(DEBUG_TEXT , "vfs::lseek" , "(next) open_offset   = %d\n" , who_opened->object->open_offset);
    return who_opened->object->open_offset;
}

    
int vfs::read_directory(file_info *file , ObjectLinkedList<char*> &file_list) {
    return 0x00;
}