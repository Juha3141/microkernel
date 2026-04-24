#include <kernel/vfs/file_system_driver.hpp>

#include <kernel/debug.hpp>

FixedArray<fsdev::file_system_driver*> *fsdriver_container;

void fsdev::init(void) {
    fsdriver_container = memory::new_global_object<FixedArray<fsdev::file_system_driver*>>();
    fsdriver_container->init(512); 
}

max_t fsdev::register_driver(fsdev::file_system_driver *driver , const char *fs_name) {
    strcpy(driver->fs_string , fs_name);
    debug::out::printf(DEBUG_TEXT , "Registered file system driver, name : \"%s\"\n" , driver->fs_string);
    return fsdriver_container->add(driver);
}

fsdev::file_system_driver *fsdev::search_driver(const char *fs_name) {
    max_t id = fsdriver_container->search(
        [fs_name](fsdev::file_system_driver *&dev) { return (bool)(strcmp(dev->fs_string , fs_name) == 0); }
    );
    return fsdriver_container->get(id);
}

fsdev::file_system_driver *fsdev::search_driver(max_t driver_id) { return fsdriver_container->get(driver_id); }

fsdev::file_system_driver *fsdev::detect_fs(blockdev::block_device *device) {
    max_t id = fsdriver_container->search(
        [device](fsdev::file_system_driver *&fdev) {
            // breakpoint
            if(fdev == 0x00) return false;
            return (bool)(fdev->check(device));
        }
    );
    if(id == INVALID) return nullptr;
    return fsdriver_container->get(id);
}

max_t fsdev::discard_driver(const char *fs_name) { return fsdriver_container->discard(search_driver(fs_name)); }
max_t fsdev::discard_driver(max_t driver_id) { return fsdriver_container->discard(search_driver(driver_id));}

physical_file_location *fsdev::get_physical_loc_info(file_info *file) {
    if(file->is_mounted == true) return &file->mount_loc_info;
    return &file->file_loc_info;
}