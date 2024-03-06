#include <kernel/virtual_file_system.hpp>
#include <kernel/file_system_driver.hpp>

#include <kernel/debug.hpp>

void vfs::init(blockdev::block_device *root_device) {
    debug::push_function("vfs::init");
    file_info *root_file = (file_info *)memory::pmem_alloc(sizeof(file_info));
    strcpy(root_file->file_name , "@");
    root_file->is_mounted = true;

    // mount the file
    if(vfs::mount(root_file , root_device) == false) {
        debug::out::printf(DEBUG_WARNING , "Failed mounting root device!\n");
        return;
    }

    debug::out::printf(DEBUG_SPECIAL , "fs_driver : 0x%lx\n" , root_file->mount_loc_info.fs_driver);
    debug::out::printf(DEBUG_SPECIAL , "Device %s%d : File system detected, %s\n" , root_device->device_driver->driver_name , root_device->id , root_file->mount_loc_info.fs_driver->fs_string);
    GLOBAL_OBJECT(VirtualFileSystemCache)->init(root_file , '/');
    debug::pop_function();
}

file_info *vfs::get_root_directory(void) { return GLOBAL_OBJECT(VirtualFileSystemCache)->fs_root_dir; }

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

file_info *vfs::open(const general_file_name file_path , int option) {
    char **file_list;
    VirtualFileSystemCache *vfs_cache = GLOBAL_OBJECT(VirtualFileSystemCache);
    int level_count = vfs_cache->auto_parse_dir_count(file_path.file_name);

    // parse the file name into an array of directory names
    file_list = (char **)memory::pmem_alloc(level_count*sizeof(char*));
    vfs_cache->auto_parse_name(file_path.file_name , file_list);

    // search for cache
    int last_hit_loc = 0;
    file_info *file = vfs_cache->search_object_last(level_count , file_path.root_directory , file_list , last_hit_loc);
    if(file == 0x00) return 0x00; // give up
    if(file != 0x00 && strcmp(file->file_name , file_list[level_count-1]) == 0) { // cash hit
        return file;
    }

    debug::out::printf("last cash hit : %s(file=0x%lx), hit_loc : %d\n" , file->file_name , file , last_hit_loc);
    
    file_info *tree_file = file;
    for(int i = last_hit_loc; i < level_count; i++) {
        physical_file_location *pfileloc = &tree_file->file_loc_info;
        if(tree_file->is_mounted == true) pfileloc = &tree_file->mount_loc_info;

        debug::out::printf("file to open : %s\n" , file_list[i]);
        debug::out::printf("root directory : %s(0x%lx)\n" , tree_file->file_name , tree_file);
        file_info *new_file_handle = pfileloc->fs_driver->get_file_handle({file_list[i] , tree_file});
        if(new_file_handle == 0x00) return 0x00;

        vfs_cache->add_object(new_file_handle , tree_file);
        tree_file = new_file_handle;
    }
    debug::out::printf(DEBUG_SPECIAL , "tree_file : 0x%X(%s,%d)\n" , tree_file , tree_file->file_name , tree_file->file_loc_info.block_location);
    // hmmm
    return tree_file;
}

int vfs::read_directory(file_info *file , ObjectLinkedList<char*> &file_list) {
    return 0x00;
}