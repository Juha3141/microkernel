#include <kernel/vfs/virtual_file_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>

#include <kernel/debug.hpp>

////// VirtualFileSystemManager

void vfs::VirtualFileSystemManager::init(file_info *rdir , blockdev::block_device *root_device , char dir_ident)  {
    fs_root_dir = rdir;
    fs_root_dir->parent_dir = 0x00;
    fs_root_dir->file_list = 0x00;
    dir_identifier = dir_ident;
    root_dev = root_device;
    is_initialized_properly = true;
}

void vfs::VirtualFileSystemManager::add_object(file_info *file , file_info *directory) {
    if(directory->file_list == 0x00) {
        directory->file_list = new LinkedList<file_info_s*>;
        directory->file_list->init();
    }
    directory->file_list->add_rear(file);
    file->parent_dir = directory;
}

bool vfs::VirtualFileSystemManager::remove_object(const char *file_name , file_info *directory) {
    if(directory->file_list == 0x00) return false;
    return directory->file_list->remove(
        directory->file_list->search<const char *>(
            [](file_info *f , const char *fn) {
                return strcmp(f->file_name , fn) == 0;
            } , file_name
        )
    );
}

/// @brief Search the file_info object
/// @param level_count 
/// @param search_root 
/// @param file_links 
/// @param last_hit_loc return  
/// @return file_info object
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
        if(ptr->file_list == 0x00) return ptr;
        LinkedList<file_info_s*>::node_s *node = ptr->file_list->search<char*>([](file_info_s *obj , char *str) { return (strcmp(obj->file_name , str) == 0); } , file_links[i]);
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

void vfs::VirtualFileSystemManager::get_file_base_name(const char *full_file_path , char *base_name) {
    int full_length = strlen(full_file_path);
    int last_ident_index = 0;
    int i = full_length-1;
    int j = 0;
    for(; i >= 0; i--) { if(full_file_path[i] == dir_identifier) { break; } }
    last_ident_index = i;
    for(i = full_length-1; i > last_ident_index; i--) { base_name[(i-last_ident_index)-1] = full_file_path[i]; }
    base_name[full_length-last_ident_index-1] = 0;
}

/// Standard vfs functions

void vfs::init(blockdev::block_device *root_device) {
    file_info *root_file = create_file_info_struct({0x00 , 0x00 , 0x00} , "@" , FILE_TYPE_DIRECTORY , 0);

    // mount the file
    if(vfs::mount(root_file , root_device) == false) {
        debug::out::printf(DEBUG_WARNING , "Failed mounting root device!\n");
        GLOBAL_OBJECT(VirtualFileSystemManager)->is_initialized_properly = false;
        return;
    }
    
    debug::out::printf(DEBUG_SPECIAL , "fs_driver : 0x%lx\n" , root_file->mount_loc_info.fs_driver);
    debug::out::printf(DEBUG_SPECIAL , "Device %s%d : File system detected, %s\n" , root_device->device_driver->driver_name , root_device->id , root_file->mount_loc_info.fs_driver->fs_string);
    GLOBAL_OBJECT(VirtualFileSystemManager)->init(root_file , root_device , '/');
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
    new_file->who_open_list = new LinkedList<open_info_t*>;
    new_file->who_open_list->init();

    new_file->file_list = 0x00;
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
    debug::out::printf(DEBUG_TEXT , "last cash hit : %s(file=0x%lx), hit_loc : %d\n" , file->file_name , file , last_hit_loc);
    
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
    open_info->cache_hash_table = new HashTable<block_cache_t , max_t>;
    open_info->new_cache_linked_list = new LinkedList<block_cache_t*>;

    open_info->new_cache_linked_list->init();
    open_info->cache_hash_table->init(512 , [](max_t&d,max_t s){d=s;} , [](max_t d,max_t s){ return (bool)(d==s); } , 
    [](max_t key) { return (hash_index_t)(key%512); });
    debug::out::printf("who_open_list : 0x%lx\n" , file->who_open_list);
    file->who_open_list->add_rear(open_info);
    return file;
}

/// @brief Flush the pre-existing caches
/// @param preexist_caches Hash table for the caches
/// @param file file_info structure
/// @return true if succeed, false if failed
static bool flush_preexisting_caches(HashTable<block_cache_t , max_t>*preexist_caches , file_info *file) {
    bool succeed = true;
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    debug::out::printf("max_index : %d\n" , preexist_caches->max_index);

    // circulate all the hash table contents
    for(int i = 0; i < preexist_caches->max_index; i++) {
        if(preexist_caches->hash_container[i].objects_container == 0x00) continue;
        LinkedList<HashTable<block_cache_t , max_t>::list_object*>*linked_lst = preexist_caches->hash_container[i].objects_container;
        LinkedList<HashTable<block_cache_t , max_t>::list_object*>::node_s *node_ptr = linked_lst->get_start_node();
        while(node_ptr != 0x00) {
            max_t block_loc = node_ptr->object->key; // key : block_loc
            if(node_ptr->object->object->flushed == true) {
                node_ptr = node_ptr->next;
                continue;
            }
            debug::out::printf("flushing preexisting cache : addr %d\n" , block_loc);
            node_ptr->object->object->flushed = true;
            if(file_loc->block_device->device_driver->write(file_loc->block_device , block_loc , 1 , node_ptr->object->object->block)
               != file_loc->block_device->geometry.block_size) node_ptr->object->object->flushed = false;

            node_ptr = node_ptr->next;
        }
    }
    return succeed;
}

/// @brief Flush the caches that are newly created
/// @param new_caches Linked list structure of newly created caches
/// @param file file_info structure
/// @param preexist_caches Hash table for the caches
/// @return true if succeed, false if failed
static bool flush_new_caches(LinkedList<block_cache_t*>*new_caches , file_info *file , open_info_t *who_opened , HashTable<block_cache_t , max_t>*preexist_caches) {
    bool succeed = true;
    max_t new_block_count = new_caches->size();
    max_t flushed_block_count = 0;
    max_t created_block_count = 0;

    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    LinkedList<block_cache_t*>::node_s *start_node = new_caches->get_start_node();
    LinkedList<block_cache_t*>::node_s *ptr = start_node;

    max_t blockdev_bs = file_loc->block_device->geometry.block_size;

    debug::out::printf("Total new caches count : %d\n" , new_block_count);

    if(new_block_count == 0) return succeed;
    while(created_block_count <= flushed_block_count) {
        max_t cluster_size = file_loc->fs_driver->get_cluster_size(file);
        max_t physical_loc = file_loc->fs_driver->allocate_new_cluster_to_file(file);
        debug::out::printf("allocated %d sectors\n" , cluster_size);
        debug::out::printf("physical location : %d\n" , physical_loc);
        for(int i = 0; i < cluster_size; i++) {
            if(ptr == 0x00) {
                break;
            }
            if(file_loc->block_device->device_driver->write(file_loc->block_device , physical_loc+i , 1 , ptr->object->block) 
                != blockdev_bs) succeed = false;
            
            LinkedList<block_cache_t*>::node_s *node_to_remove = ptr;
            ptr = ptr->next;

            // Add the flushed node to the pre-existing hash table, remove from new cache list. 
            node_to_remove->object->flushed = true;
            preexist_caches->add(physical_loc+i , node_to_remove->object);
            new_caches->remove(node_to_remove);
            debug::out::printf("writing the cache data to %d\n" , physical_loc+i);
        }
        if(ptr == 0x00) break;
        created_block_count += cluster_size;
    }

    return succeed;
}

bool vfs::flush(file_info *file) {
    max_t task_id;

    task_id = 0x00; // not implemented yet!
    LinkedList<open_info_t*>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj,max_t id) {return obj->task_id==id;} , task_id);
    if(who_opened == 0x00) return false;

    HashTable<block_cache_t , max_t>*preexist_cache_hash_table = who_opened->object->cache_hash_table;
    LinkedList<block_cache_t*>*new_cache_linked_list = who_opened->object->new_cache_linked_list;
    if(flush_preexisting_caches(preexist_cache_hash_table , file) == false) return false;
    if(flush_new_caches(new_cache_linked_list , file , who_opened->object , preexist_cache_hash_table) == false) return false;

    return true;
}

bool vfs::close(file_info *file) {
    max_t current_task_id = 0x00; // not implemented!
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    if(vfs::flush(file) == false) return false; // flush the file

    LinkedList<open_info_t*>::node_s *node = file->who_open_list->search<max_t>([](open_info_t *o,max_t id) {return(bool)(o->task_id==id);} , current_task_id);
    if(node->object->maximum_offset > file->file_size) {
        debug::out::printf("applying new file info\n");
        file_loc->fs_driver->apply_new_file_info(file , node->object->maximum_offset);

        file->file_size = node->object->maximum_offset;
    }
    
    // To-do : Free the hash table and linked list
    file->who_open_list->remove(node);
    return true;
}

bool vfs::remove(const general_file_name file_path) {
    file_info *parent_dir = get_file_by_cache_and_phys(file_path , 1);
    physical_file_location *parent_loc = fsdev::get_physical_loc_info(parent_dir);
    VirtualFileSystemManager *vfs_mgr = GLOBAL_OBJECT(VirtualFileSystemManager);

    if(parent_dir == 0x00) return false;
    char base_file_name[strlen(file_path.file_name)+2];
    vfs_mgr->get_file_base_name(file_path.file_name , base_file_name);

    debug::out::printf("base_file_name : %s\n" , base_file_name);

    if(parent_loc->fs_driver->remove({base_file_name , parent_dir}) == false) return false;
    vfs_mgr->remove_object(base_file_name , parent_dir);

    return true;
}

bool vfs::rename(const general_file_name file_path , const char *new_name) {
    return false;
}

bool vfs::move(const general_file_name file_path , const general_file_name new_directory) {
    return 0x00;
}

/// @brief Get the cache object from the cache storage. New cache object is automatically created when the cache does not exist. 
/// @param file File handle
/// @param linear_block_addr Linear block address
/// @param open_info Information about file open 
/// @return block cache object
static block_cache_t *get_cache_data(file_info *file , max_t linear_block_addr , open_info_t *open_info) {
    block_cache_t *cache;

    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    max_t block_size = file_loc->block_device->geometry.block_size;
    max_t cluster_size = file_loc->fs_driver->get_cluster_size(file);
    max_t cluster_start_phys_location = file_loc->fs_driver->get_cluster_start_address(file , linear_block_addr);
    if(cluster_start_phys_location == INVALID) {
        // Out of the bounds, search from new_cache_linked_list
        LinkedList<block_cache_t*>::node_s *n = open_info->new_cache_linked_list->search<max_t>([](block_cache_t *o,max_t l){return o->linear_block_addr == l;} , linear_block_addr);
        if(n != 0x00) return n->object;
        return 0x00;
    }
    
    // actual physical location of the block
    max_t block_phys_location = cluster_start_phys_location+(linear_block_addr%cluster_size);

    cache = open_info->cache_hash_table->search(block_phys_location);
    if(cache != 0x00) return cache;

    LinkedList<block_cache_t*>::node_s *n = open_info->new_cache_linked_list->search<max_t>([](block_cache_t *o,max_t l){return o->linear_block_addr == l;} , linear_block_addr);
    if(n != 0x00) return n->object;
    // we actually need to create new cache page now..

    // cache does not exist, create new cache
    debug::out::printf("cluster location : %d\n" , cluster_start_phys_location);
    
    unsigned char *temp_buffer = (unsigned char *)memory::pmem_alloc(cluster_size*block_size);
    file_loc->block_device->device_driver->read(file_loc->block_device , cluster_start_phys_location , cluster_size , temp_buffer);
    
    block_cache_t *caches_ptr[cluster_size];
    for(max_t i = 0; i < cluster_size; i++) {
        cache = (block_cache_t *)memory::pmem_alloc(sizeof(block_cache_t));
        cache->block = (void *)memory::pmem_alloc(block_size);

        memcpy(cache->block , temp_buffer+(i*block_size) , block_size);

        cache->block_size = block_size;
        cache->flushed = true; // newest version.

        // add to the cache table
        open_info->cache_hash_table->add(cluster_start_phys_location+i , cache);
        // block device as a unit
        caches_ptr[i] = cache;
    }

    memory::pmem_free(temp_buffer);
    return caches_ptr[linear_block_addr%cluster_size];
}

long vfs::read(file_info *file , max_t size , void *buffer) {
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
    
    LinkedList<open_info_t*>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
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
        caches[b-block_start] = get_cache_data(file , b , who_opened->object);
        debug::out::printf("cache : 0x%X\n" , caches[b-block_start]);
        max_t boff = off%block_size;
        max_t bsize = MIN(end_offset-off , block_size-boff);
        memcpy(buffer+buffer_offset , caches[b-block_start]->block+boff , bsize);
        buffer_offset += bsize;
        off += bsize;
        read_size += bsize;
    }
    who_opened->object->open_offset += read_size;
    debug::out::printf("read_size : %d\n" , read_size);
    return read_size;
}

long vfs::write(file_info *file , max_t size , const void *buffer) {
    max_t current_task_id;

    max_t open_offset = 0;
    max_t block_size;

    max_t block_start;
    max_t block_end;
    max_t block_count;
    max_t block_file_end;
    max_t required_block_count;

    physical_file_location *file_loc;
    if(file->who_open_list == 0x00) return 0; // error
    current_task_id = 0x00; // currently not implemented yet!
    
    LinkedList<open_info_t*>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
    if(who_opened == 0x00) return 0;

    file_loc = fsdev::get_physical_loc_info(file);

    // fill out basic informations
    block_size = file_loc->block_device->geometry.block_size;
    open_offset = who_opened->object->open_offset;

    block_file_end = who_opened->object->maximum_offset/block_size;
    
    // block_start, block_end , block_count : region that will be written
    block_start = open_offset/block_size;
    block_end = (open_offset+size)/block_size;
    block_count = block_end-block_start+1;
    
    // If the end of the target blocks(writing) exceeds the current maximum file --> create new block
    required_block_count = (block_end > block_file_end) ? block_end-block_file_end : 0;

    debug::out::printf("file_size   = %d\n" , who_opened->object->maximum_offset);
    debug::out::printf("open_offset = %d\n" , open_offset);
    debug::out::printf("size        = %d\n" , size);
    debug::out::printf("req_bcount  = %d\n" , required_block_count);
    // Create new block
    if(required_block_count != 0) {
        // Register new blocks to the "who_opened" structure
        for(max_t i = 1; i <= required_block_count; i++) {
            max_t linear_block_addr = block_file_end+i;
            block_cache_t *oldone_exist;
            oldone_exist = get_cache_data(file , linear_block_addr , who_opened->object);
            if(oldone_exist != 0x00) continue;

            // allocate new cache structure
            block_cache_t *cache = (block_cache_t *)memory::pmem_alloc(sizeof(block_cache_t));
            
            // allocate the memory space for the cache
            cache->block = (void *)memory::pmem_alloc(block_size);
            memset(cache->block , 0 , block_size);
            
            cache->block_size = block_size;
            cache->flushed = true; // newest
            cache->linear_block_addr = linear_block_addr;
            
            who_opened->object->new_cache_linked_list->add_rear(cache);
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
        caches[b-block_start] = get_cache_data(file , b , who_opened->object);
        
        caches[b-block_start]->flushed = false;

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
    return write_size;
}


long vfs::lseek(file_info *file , long cursor , int option) {
    max_t current_task_id;
    current_task_id = 0x00; /* Not implemented */

    LinkedList<open_info_t*>::node_s *who_opened = file->who_open_list->search<max_t>([](open_info_t *obj , max_t tid) { return (bool)(obj->task_id == tid); } , current_task_id);
    if(who_opened == 0x00) return -1;
    debug::out::printf(DEBUG_TEXT , "(before) open_offset = %d\n" , who_opened->object->open_offset);
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
    debug::out::printf(DEBUG_TEXT , "(next) open_offset   = %d\n" , who_opened->object->open_offset);
    return who_opened->object->open_offset;
}

static void discard_file_info(file_info *file) {
    if(file->who_open_list != 0x00) memory::pmem_free(file->who_open_list);
    if(file->file_list != 0x00) memory::pmem_free(file->file_list);
    memory::pmem_free(file);
}

/// @brief Read the files in the directory and store the file_info structure to the "file_list" cache.
///        This function does not return any sort of list or file names. Instead, it stores the list of file into the
///        file_list structure in file_info structure. 
/// @param root_directory file_info of the directory
/// @return File count
int vfs::read_directory(file_info *directory) {
    LinkedList<file_info*> file_info_list;
    file_info_list.init();
    physical_file_location *file_loc = fsdev::get_physical_loc_info(directory);
    if(file_loc == 0x00) return -1;

    int file_count = file_loc->fs_driver->read_directory(directory , file_info_list);
    auto *ptr = file_info_list.get_start_node();

    if(directory->file_list == 0x00) {
        directory->file_list = new LinkedList<file_info_s*>;
        directory->file_list->init();
    }
    while(ptr != 0x00) {
        file_info *new_file = ptr->object;
        LinkedList<file_info*>::node_s *file_node = directory->file_list->search<const char *>(
            [](file_info *o , const char *fn) { return (strcmp(o->file_name , fn) == 0); } , 
            new_file->file_name
        );
        
        // does not exist, register new one
        if(file_node == 0x00) {
            new_file->parent_dir = directory;
            directory->file_list->add_rear(new_file);
        }
        // already exist, just discard
        else {
            discard_file_info(new_file);
        }

        ptr = ptr->next;
    }
    
    return file_count;
}