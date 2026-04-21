#include <kernel/vfs/file_system_driver.hpp>

#include <kernel/debug.hpp>

void fsdev::init(void) { GLOBAL_OBJECT(FileSystemDriverContainer)->init(512); }

max_t fsdev::register_driver(fsdev::file_system_driver *driver , const char *fs_name) {
    strcpy(driver->fs_string , fs_name);
    debug::out::printf(DEBUG_TEXT , "Registered file system driver, name : \"%s\"\n" , driver->fs_string);
    return GLOBAL_OBJECT(FileSystemDriverContainer)->add(driver);
}

fsdev::file_system_driver *fsdev::search_driver(const char *fs_name) {
    max_t id = GLOBAL_OBJECT(FileSystemDriverContainer)->search(
        [fs_name](fsdev::file_system_driver *&dev) { return (bool)(strcmp(dev->fs_string , fs_name) == 0); }
    );
    return GLOBAL_OBJECT(FileSystemDriverContainer)->get(id);
}

fsdev::file_system_driver *fsdev::search_driver(max_t driver_id) { return GLOBAL_OBJECT(FileSystemDriverContainer)->get(driver_id); }

fsdev::file_system_driver *fsdev::detect_fs(blockdev::block_device *device) {
    max_t id = GLOBAL_OBJECT(FileSystemDriverContainer)->search(
        [device](fsdev::file_system_driver *&fdev) {
            // breakpoint
            if(fdev == 0x00) return false;
            return (bool)(fdev->check(device));
        }
    );
    if(id == INVALID) return nullptr;
    return GLOBAL_OBJECT(FileSystemDriverContainer)->get(id);
}

max_t fsdev::discard_driver(const char *fs_name) { return GLOBAL_OBJECT(FileSystemDriverContainer)->discard(search_driver(fs_name)); }
max_t fsdev::discard_driver(max_t driver_id) { return GLOBAL_OBJECT(FileSystemDriverContainer)->discard(search_driver(driver_id));}

physical_file_location *fsdev::get_physical_loc_info(file_info *file) {
    if(file->is_mounted == true) return &file->mount_loc_info;
    return &file->file_loc_info;
}