#include <kernel/vfs/virtual_file_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>

#include <kernel/debug.hpp>

vfs::VirtualFileSystemManager *vfs_mgr;

////// VirtualFileSystemManager

void vfs::VirtualFileSystemManager::init(file_t *rdir , block_device *root_device , char dir_ident)  {
    fs_root_dir = rdir;
    fs_root_dir->info->parent_dir = 0x00;
    fs_root_dir->info->file_list = 0x00;
    dir_identifier = dir_ident;
    root_dev = root_device;
    is_initialized_properly = true;
}

void vfs::VirtualFileSystemManager::add_object(file_t *file , file_t *directory) {
    if(directory->info->file_list == 0x00) {
        directory->info->file_list = new LinkedList<file_t*>;
        directory->info->file_list->init();
    }
    directory->info->file_list->add_rear(file);
    file->info->parent_dir = directory;
}

bool vfs::VirtualFileSystemManager::remove_object(const char *file_name , file_t *directory) {
    if(directory->info->file_list == 0x00) return false;
    return directory->info->file_list->remove(
        directory->info->file_list->search(
            [file_name](file_t *f) { return strcmp(f->name , file_name) == 0; }
        )
    );
}

/// @brief Search the file_info object
/// @param level_count 
/// @param search_root 
/// @param file_links 
/// @param last_hit_loc return  
/// @return file_info object
file_t *vfs::VirtualFileSystemManager::search_object_last(int level_count , file_t *search_root , char **file_links , int &last_hit_loc) {
    file_t *ptr = search_root;
    int i = 0;
    // If file links start with root directory
    if(strcmp(file_links[0] , fs_root_dir->name) == 0) {
        // root path is root directory
        ptr = fs_root_dir;
        i++;
    }
    for(; i < level_count; i++) {
        last_hit_loc = i;
        if(ptr->info->file_list == 0x00) return ptr;
        LinkedList<file_t*>::node_s *node = ptr->info->file_list->search(
            [file_links,i](file_t *obj) { return (strcmp(obj->name , file_links[i]) == 0); }
        );
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

void vfs::init(block_device *root_device) {
    file_info *root_file_info = create_file_info_struct({0 , nullptr , nullptr} , "@" , FILE_TYPE_DIRECTORY , 0 , nullptr);
    file_t *root_file = create_file_struct("@" , root_file_info);

    vfs_mgr = memory::new_global_object<VirtualFileSystemManager>();
    // mount the file
    if(vfs::mount(root_file , root_device) == false) {
        debug::out::printf(DEBUG_WARNING , "Failed mounting root device!\n");
        vfs_mgr->is_initialized_properly = false;
        return;
    }
    
    debug::out::printf(DEBUG_SPECIAL , "fs_driver : 0x%lx\n" , root_file->info->mount_loc_info.fs_driver);
    debug::out::printf(DEBUG_SPECIAL , "Device %s%d" , root_device->driver->driver_name , root_device->id);
    if(root_device->storage_info.storage_type == storage_logical) {
        debug::out::printf(DEBUG_SPECIAL , "part%d" , root_device->storage_info.partition_id);
    }
    debug::out::printf(DEBUG_SPECIAL , " : File system detected, %s\n" , root_file->info->mount_loc_info.fs_driver->fs_string);
    vfs_mgr->init(root_file , root_device , '/');
}

file_t *vfs::get_root_directory(void) { return vfs_mgr->fs_root_dir; }

file_t *vfs::create_file_struct(const char *file_name , file_info *info) {
    file_t *new_file = new file_t;
    // file name
    strcpy(new_file->name , file_name);
    
    new_file->is_symlink = false;
    new_file->mounted_dir_info = nullptr;
    new_file->is_mounted_unimplemented = false;
    new_file->symlink_dir_info = nullptr;

    new_file->info = info;
    
    return new_file;
}

file_info *vfs::create_file_info_struct(
        const physical_file_location file_loc , 
        const char *phys_file_name , 
        int file_type ,
        max_t file_size , 
        file_t *parent_dir) {
    file_info *new_info = new file_info;
    memset(new_info , 0 , sizeof(file_info));
    // mount flag
    new_info->is_mounted = false;
    // file physical location
    new_info->file_loc_info.block_device   = file_loc.block_device;
    new_info->file_loc_info.block_location = file_loc.block_location;
    new_info->file_loc_info.fs_driver      = file_loc.fs_driver;

    // file physical name
    strcpy(new_info->physical_name , phys_file_name);

    // file type
    new_info->file_type = file_type;

    new_info->file_size = file_size;
    new_info->who_open_list = new LinkedList<open_info_t*>;
    new_info->who_open_list->init();

    new_info->file_list = nullptr;
    // parent directory
    new_info->parent_dir = parent_dir;
    return new_info;
}

bool vfs::mount(file_t *file , block_device *device) {
    fsdev::file_system_driver *fs_driver = device->storage_info.fs_driver;
    if(device->storage_info.fs_driver == nullptr) return false;
    
    file->info->mount_loc_info.fs_driver = fs_driver;
    file->info->mount_loc_info.block_device = device;
    if(fs_driver->get_root_directory(file->info->mount_loc_info) == false) return false; 

    file->info->is_mounted = true;
    return true;
}

/// @brief Given a name with directory, give the name at the top directory
///        E.g. if the given name is "/test/hello/world", output will be "world".
///        (Directory identifier in the example is '/')
/// @param original_name Original name with directory paths included
/// @param output 
static void get_highest_level_file_name(const char *original_name , char *output) {
    int len = strlen(original_name);
    char identifier = vfs_mgr->dir_identifier;
    char *str;

    int i = len-1;
    for(; i >= 0; i--) {
        if(original_name[i] == identifier) { break; }
    }
    i++;
    
    memcpy(output , original_name+i , len-i);
    output[len-i] = 0x00;
}

/// @brief Automatically make a space for file names, parse the file name and store it to the allocated space
// E.g.    parse "dir1/dir2/dir3/file" to {"dir1","dir2","dir3","file"}
///        Important! Please discard the memory after using the file list
/// @param file_path File path to be parsed
/// @param file_list the resulting file list
/// @return Number of parsed names
static int get_file_name_list(const general_file_name file_path , char **(&file_list)) {
    int lvl_count = vfs_mgr->auto_parse_dir_count(file_path.file_name);
    file_list = (char **)memory::pmem_alloc(lvl_count*sizeof(char*));
    vfs_mgr->auto_parse_name(file_path.file_name , file_list);
    return lvl_count;
}

/// @brief Check whether "." and ".." directory exists in the given file handle
///        Link them properly to right file handles
/// @param directory 
static void handle_dot_directories(file_t *directory) {
    LinkedList<file_t*>*file_list = directory->info->file_list;
    auto ptr = file_list->get_start_node();
    bool dot_exists = false;
    bool dotdot_exists = false;
    while(ptr != nullptr) {
        if(strcmp(ptr->object->name , ".") == 0) {
            delete ptr->object->info;
            ptr->object->info = directory->info;
            dot_exists = true;
        } 
        else if(strcmp(ptr->object->name , "..") == 0) {
            delete ptr->object->info;
            ptr->object->info = directory->info->parent_dir->info;
            dotdot_exists = true;
        }

        ptr = ptr->next;
    }

    if(!dotdot_exists) {
        // If this is root directory, ".." points to itself
        file_t *file = vfs::create_file_struct(".." , directory->info->parent_dir ? directory->info->parent_dir->info : directory->info);
        file_list->add_front(file);
    }

    if(!dot_exists) {
        file_t *file = vfs::create_file_struct("." , directory->info);
        file_list->add_front(file);
    }
}

/// @brief Get the file_info handle by using cache. 
///        If there is no file on the cache tree, function physically searches for the file.
///        The files opened for directory searching is stored into cache tree. 
/// @param file_path General file path
/// @param levels_to_exclude Number of directory levels to exclude from searching
/// @return file_info handle, if failed searching(file does not exist), return 0x00
static file_t *get_file_by_cache_and_phys(const general_file_name file_path , int levels_to_exclude) {
    char **file_list;
    int level_count;
    int last_hit_loc = 0;

    // failed to initialize the vfs manager
    if(!vfs_mgr->is_initialized_properly) return nullptr;
    if(file_path.root_directory == nullptr && strlen(file_path.file_name) == 0) return nullptr;

    level_count = get_file_name_list(file_path , file_list);

    file_t *file = vfs_mgr->search_object_last(level_count , file_path.root_directory , file_list , last_hit_loc);
    
    // file does not exist
    if(file == nullptr) return nullptr;
    if(file != 0x00 && strcmp(file->name , file_list[level_count-1]) == 0) { // cash hit
        for(int i = 0; i < level_count; i++) {
            memory::pmem_free(file_list[i]);
        }
        memory::pmem_free(file_list);

        return file;
    }
    debug::out::printf(DEBUG_TEXT , "last cash hit : %s(file=0x%llx), hit_loc : %d\n" , file->name , file , last_hit_loc);
    
    file_t *tree_file = file;
    for(int i = last_hit_loc; i < level_count-levels_to_exclude; i++) {
        debug::out::printf("file_list[i] = %s\n" , file_list[i]);
        physical_file_location *pfileloc = fsdev::get_physical_loc_info(file);

        LinkedList<file_t*>* &file_t_list = tree_file->info->file_list;
        if(file_t_list == nullptr) {
            file_t_list = new LinkedList<file_t*>;
            pfileloc->fs_driver->read_directory(file , *file_t_list);
            handle_dot_directories(file);
        }
        
        auto ptr = file_t_list->get_start_node();
        bool found = false;
        while(ptr != nullptr) {
            if(strcmp(ptr->object->name , file_list[i]) == 0) {
                tree_file = ptr->object;
                found = true;
                break;
            }
            ptr = ptr->next;
        }
        if(!found) {
            tree_file = nullptr;
            break;
        }
    }


    for(int i = 0; i < level_count; i++) {
        memory::pmem_free(file_list[i]);
    }
    memory::pmem_free(file_list);
    return tree_file;
}

// Warning: this function used to have variable length string. The string is temporarily replaced with pmem allocated object, 
// until an allocator that reduces the memory fragmentation in this kind of situation is implemented.
bool vfs::create(const general_file_name file_path , word file_type) {
    file_t *directory;
    physical_file_location *physical_loc;
    char *temp_name = (char *)memory::pmem_alloc(strlen(file_path.file_name)+1);

    directory = get_file_by_cache_and_phys(file_path , 1);
        if(directory == 0x00) {
        memory::pmem_free(temp_name);
        return false;
    }

    physical_loc = fsdev::get_physical_loc_info(directory);
    get_highest_level_file_name(file_path.file_name , temp_name);

    if(physical_loc->fs_driver == 0x00) {
        memory::pmem_free(temp_name);
        return false;
    }
    bool res = physical_loc->fs_driver->create({temp_name , directory} , file_type);
    memory::pmem_free(temp_name);
    return res;
}

file_t *vfs::open(const general_file_name file_path , int option) {
    open_info_t *open_info = (open_info_t *)memory::pmem_alloc(sizeof(open_info_t));
    file_t *file = get_file_by_cache_and_phys(file_path , 0);
    max_t current_task_id = 0x00; // currently not implemented yet!
    if(file == 0x00) return 0x00;

    open_info->maximum_offset = file->info->file_size;
    open_info->open_offset = 0;
    open_info->open_flag = option;
    open_info->task_id = current_task_id;
    open_info->cache_hash_table = new HashTable<block_cache_t , max_t>;
    open_info->new_cache_linked_list = new LinkedList<block_cache_t*>;

    open_info->new_cache_linked_list->init();
    open_info->cache_hash_table->init(512 , [](max_t&d,max_t s){d=s;} , [](max_t d,max_t s){ return (bool)(d==s); } , 
    [](max_t key) { return (hash_index_t)(key%512); });
    file->info->who_open_list->add_rear(open_info);
    return file;
}

/// @brief Flush the pre-existing caches
/// @param preexist_caches Hash table for the caches
/// @param file file_info structure
/// @return true if succeed, false if failed
static bool flush_preexisting_caches(HashTable<block_cache_t , max_t>*preexist_caches , file_t *file) {
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
            if(file_loc->block_device->driver->write(file_loc->block_device , block_loc , 1 , node_ptr->object->object->block)
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
static bool flush_new_caches(LinkedList<block_cache_t*>*new_caches , file_t *file , open_info_t *who_opened , HashTable<block_cache_t , max_t>*preexist_caches) {
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
            if(file_loc->block_device->driver->write(file_loc->block_device , physical_loc+i , 1 , ptr->object->block) 
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

bool vfs::flush(file_t *file) {
    max_t task_id;
    if(file->info == nullptr||file->info->who_open_list == nullptr) return false;

    task_id = 0x00; // not implemented yet!
    LinkedList<open_info_t*>::node_s *who_opened = file->info->who_open_list->search(
        [task_id](open_info_t *obj) { return obj->task_id==task_id; }
    );
    if(who_opened == 0x00) return false;

    HashTable<block_cache_t , max_t>*preexist_cache_hash_table = who_opened->object->cache_hash_table;
    LinkedList<block_cache_t*>*new_cache_linked_list = who_opened->object->new_cache_linked_list;
    if(flush_preexisting_caches(preexist_cache_hash_table , file) == false) return false;
    if(flush_new_caches(new_cache_linked_list , file , who_opened->object , preexist_cache_hash_table) == false) return false;

    return true;
}

bool vfs::close(file_t *file) {
    max_t current_task_id = 0x00; // not implemented!
    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    if(vfs::flush(file) == false) return false; // flush the file

    LinkedList<open_info_t*>::node_s *node = file->info->who_open_list->search(
        [current_task_id](open_info_t *o) { return(bool)(o->task_id==current_task_id); }
    );
    if(node->object->maximum_offset > file->info->file_size) {
        debug::out::printf("applying new file info\n");
        file_loc->fs_driver->apply_new_file_info(file , node->object->maximum_offset);

        file->info->file_size = node->object->maximum_offset;
    }
    
    // To-do : Free the hash table and linked list
    file->info->who_open_list->remove(node);
    return true;
}

bool vfs::remove(const general_file_name file_path) {
    file_t *parent_dir = get_file_by_cache_and_phys(file_path , 1);
    physical_file_location *parent_loc = fsdev::get_physical_loc_info(parent_dir);

    if(parent_dir == 0x00) return false;
    char *base_file_name = (char *)memory::pmem_alloc(strlen(file_path.file_name)+2);
    vfs_mgr->get_file_base_name(file_path.file_name , base_file_name);

    debug::out::printf("base_file_name : %s\n" , base_file_name);

    if(parent_loc->fs_driver->remove({base_file_name , parent_dir}) == false) {
        memory::pmem_free(base_file_name);
        return false;
    }
    vfs_mgr->remove_object(base_file_name , parent_dir);

    memory::pmem_free(base_file_name);
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
static block_cache_t *get_cache_data(file_t *file , max_t linear_block_addr , open_info_t *open_info) {
    block_cache_t *cache;

    physical_file_location *file_loc = fsdev::get_physical_loc_info(file);
    max_t block_size = file_loc->block_device->geometry.block_size;
    max_t cluster_size = file_loc->fs_driver->get_cluster_size(file);
    max_t cluster_start_phys_location = file_loc->fs_driver->get_cluster_start_address(file , linear_block_addr);
    if(cluster_start_phys_location == INVALID) {
        // Out of the bounds, search from new_cache_linked_list
        LinkedList<block_cache_t*>::node_s *n = open_info->new_cache_linked_list->search(
            [linear_block_addr](block_cache_t *o) { return o->linear_block_addr == linear_block_addr; }
        );
        if(n != 0x00) return n->object;
        return 0x00;
    }
    
    // actual physical location of the block
    max_t block_phys_location = cluster_start_phys_location+(linear_block_addr%cluster_size);
    debug::out::printf("block_phys_location = %lld\n" , block_phys_location);
    cache = open_info->cache_hash_table->search(block_phys_location);
    debug::out::printf("cache found from hash table = 0x%llx\n" , cache);
    if(cache != 0x00) return cache;

    LinkedList<block_cache_t*>::node_s *n = open_info->new_cache_linked_list->search(
        [linear_block_addr](block_cache_t *o) { return o->linear_block_addr == linear_block_addr; }
    );
    if(n != 0x00) return n->object;
    // we actually need to create new cache page now..
    
    unsigned char *temp_buffer = (unsigned char *)memory::pmem_alloc(cluster_size*block_size);
    debug::out::printf("temp_buffer size = %lld\n" , cluster_size*block_size);
    file_loc->block_device->driver->read(file_loc->block_device , cluster_start_phys_location , cluster_size , temp_buffer);
    
    block_cache_t **caches_ptr = (block_cache_t **)memory::pmem_alloc(cluster_size*sizeof(block_cache_t *));
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

    block_cache_t *ptr_ret = caches_ptr[linear_block_addr%cluster_size];
    memory::pmem_free(temp_buffer);
    memory::pmem_free(caches_ptr);
    return ptr_ret;
}

long vfs::read(file_t *file , max_t size , void *buffer) {
    max_t current_task_id;

    max_t open_offset = 0;
    max_t end_offset = 0;
    max_t block_size;
    max_t block_start;
    max_t block_count;
    debug::disable();
    physical_file_location *file_loc;
    if(file->info == nullptr||file->info->who_open_list == nullptr) return 0; // error
    current_task_id = 0x00; // currently not implemented yet!
    
    LinkedList<open_info_t*>::node_s *who_opened = file->info->who_open_list->search(
        [current_task_id](open_info_t *obj) { return (bool)(obj->task_id == current_task_id); }
    );
    if(who_opened == nullptr) return 0; // If not opened -- 

    file_loc = fsdev::get_physical_loc_info(file);

    // fill out basic informations
    block_size = file_loc->block_device->geometry.block_size;
    open_offset = who_opened->object->open_offset;
    end_offset = min(who_opened->object->maximum_offset , open_offset+size);
    block_start = open_offset/block_size;
    block_count = size/block_size + (((open_offset+size)%block_size == 0) ? 0 : 1);

    debug::out::printf("block_size : %d\n" , block_size);

    debug::out::printf("read size : %d\n" , end_offset-open_offset);

    debug::out::printf("open_offset : %d\n" , open_offset);
    debug::out::printf("block_start : %ld\n" , block_start);
    debug::out::printf("block_count : %ld\n" , block_count);

    block_cache_t **caches = (block_cache_t **)memory::pmem_alloc(block_count*sizeof(block_cache_t*));

    // Calculate & Copy
    max_t off = open_offset;
    max_t buffer_offset = 0; 
    max_t read_size = 0;
    for(max_t b = block_start; b < block_start+block_count; b++) {
        caches[b-block_start] = get_cache_data(file , b , who_opened->object);
        debug::out::printf("cache : 0x%llx\n" , caches[b-block_start]);
        debug::out::printf("   block    = 0x%llx\n" , caches[b-block_start]->block);
        debug::out::printf("   block_sz = %lld\n" , caches[b-block_start]->block_size);
        debug::out::printf("   flushed  = %lld\n" , caches[b-block_start]->flushed);
        debug::out::printf("   lba      = %lld\n" , caches[b-block_start]->linear_block_addr);
        max_t boff = off%block_size;
        max_t bsize = min(end_offset-off , block_size-boff);
        memcpy((void *)((max_t)buffer+buffer_offset) , (void *)((max_t)caches[b-block_start]->block+boff) , bsize);
        buffer_offset += bsize;
        off += bsize;
        read_size += bsize;
    }
    who_opened->object->open_offset += read_size;
    debug::out::printf("read_size : %d\n" , read_size);

    memory::pmem_free(caches);
    debug::enable();
    return read_size;
}

long vfs::write(file_t *file , max_t size , const void *buffer) {
    max_t current_task_id;

    max_t open_offset = 0;
    max_t block_size;

    max_t block_start;
    max_t block_end;
    max_t block_count;
    max_t block_file_end;
    max_t required_block_count;

    physical_file_location *file_loc;
    if(file->info == nullptr||file->info->who_open_list == nullptr) return 0; // error
    current_task_id = 0x00; // currently not implemented yet!
    
    LinkedList<open_info_t*>::node_s *who_opened = file->info->who_open_list->search(
        [current_task_id](open_info_t *obj) { return (bool)(obj->task_id == current_task_id); }
    );
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
    block_cache_t **caches = (block_cache_t **)memory::pmem_alloc(block_count*sizeof(block_cache_t *));
    debug::out::printf("block_start : %d\n" , block_start);
    debug::out::printf("block_end   : %d\n" , block_end);
    for(max_t b = block_start; b <= block_end; b++) {
        caches[b-block_start] = get_cache_data(file , b , who_opened->object);
        
        caches[b-block_start]->flushed = false;

        max_t boff = off%block_size;
        max_t bsize = min((open_offset+size)-off , block_size-boff);
        debug::out::printf("cache : 0x%X\n" , caches[b-block_start]);
        debug::out::printf("buffer_offset = %d\n" , buffer_offset);
        debug::out::printf("block_start   = %d\n" , block_start);
        memcpy((void *)(((max_t)caches[b-block_start]->block)+boff) , (void *)((max_t)buffer+buffer_offset) , bsize);

        buffer_offset += bsize;
        off += bsize;
        write_size += bsize;
    }
    debug::out::printf("write_size : %d\n" , write_size);
    who_opened->object->open_offset += write_size;
    who_opened->object->maximum_offset = max(who_opened->object->open_offset , who_opened->object->maximum_offset);
    memory::pmem_free(caches);
    return write_size;
}


long vfs::lseek(file_t *file , long cursor , int option) {
    max_t current_task_id;
    current_task_id = 0x00; /* Not implemented */

    LinkedList<open_info_t*>::node_s *who_opened = file->info->who_open_list->search(
        [current_task_id](open_info_t *obj) { return (bool)(obj->task_id == current_task_id); }
    );
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
    who_opened->object->open_offset = min(who_opened->object->open_offset , who_opened->object->maximum_offset);
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
///        IMPORTANT : 
///        Keep in note that the "file_list" in file_info structure is automatically updated whenever changes in file occurs. 
/// @param root_directory file_info of the directory
/// @return File count
int vfs::read_directory(file_t *directory) {
    if(directory == nullptr) return -1;
    if(directory->info->file_type != FILE_TYPE_DIRECTORY) return -1;
    
    LinkedList<file_t*> *file_list;
    physical_file_location *file_loc = fsdev::get_physical_loc_info(directory->info);
    if(file_loc == 0x00) return -1;

    // if the file_list already exists, do nothing
    if(directory->info->file_list != nullptr) {
        return directory->info->file_list->size();
    }

    file_list = new LinkedList<file_t*>;
    file_list->init();

    if(file_loc->fs_driver == nullptr) {
        debug::out::printf(DEBUG_ERROR , "file_loc invalid! file_loc->fs_driver = nullptr\n");
        return -1;
    }
    int file_count = file_loc->fs_driver->read_directory(directory , *file_list);
    auto *ptr = file_list->get_start_node();

    directory->info->file_list = file_list;
    handle_dot_directories(directory);
    
    return file_count;
}

static void reverse_string(char *str) {
    int len = strlen(str);
    for(int i = 0; i < len/2; i++) {
        char t = str[len-1-i];
        str[len-1-i] = str[i];
        str[i] = t;
    }
}

void vfs::get_full_filename(file_t *file , String& filename) {
    file_t *ptr = file;
    if(ptr == get_root_directory()) {
        filename = ptr->info->physical_name;
        return;
    }
    filename.clear();
    while(ptr != nullptr) {
        String tmp_str(ptr->info->physical_name);
        reverse_string(tmp_str.c_str());

        filename += tmp_str;
        filename += vfs_mgr->dir_identifier;
        ptr = ptr->info->parent_dir;
    }
    filename.backspace();
    reverse_string(filename.c_str());
}