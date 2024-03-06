#ifndef _FILE_SYSTEM_DRIVER_HPP_
#define _FILE_SYSTEM_DRIVER_HPP_

#include <kernel/device_driver.hpp>
#include <kernel/block_device_driver.hpp>
#include <kernel/virtual_file_system.hpp>
#include <object_manager.hpp>

#define FILE_OPEN_READONLY   0x01
#define FILE_OPEN_WRITEONLY  0x02
#define FILE_OPEN_APPEND     0x04
#define FILE_OPEN_ERASE      0x08
#define FILE_OPEN_RW         0x10

#define LSEEK_SET 1
#define LSEEK_CUR 2
#define LSEEK_END 3

namespace fsdev {
    // I don't know why, but for some whatever reason weird error about vtable occurs
    // .. when declared class as abstract class..
    struct file_system_driver {
        bool (*check)(blockdev::block_device *device);
        bool (*get_root_directory)(physical_file_location &file_loc);

        bool (*create)(const general_file_name file_name , file_info *directory);

        file_info *(*get_file_handle)(const general_file_name file_name);

        bool (*remove)(const general_file_name file_name);

        bool (*rename)(const general_file_name file_name);
        bool (*move)(const general_file_name file_name , file_info *new_directory);

        int (*read)(file_info *file , size_t size , void *buffer);
        int (*write)(file_info *file , size_t size , const void *buffer);

        int (*lseek)(file_info *file , max_t cursor , int option);

        int (*read_directory)(file_info *file , max_t cursor);

        char fs_string[32];
    };

    struct FileSystemDriverContainer : public ObjectManager<file_system_driver> {
        SINGLETON_PATTERN_PMEM(FileSystemDriverContainer);
    };
    void init(void);

    max_t register_driver(file_system_driver *driver , const char *fs_name);
    file_system_driver *search_driver(const char *fs_name);
    file_system_driver *search_driver(max_t driver_id);

    file_system_driver *detect_fs(blockdev::block_device *device);

    max_t discard_driver(const char *fs_name);
    max_t discard_driver(max_t driver_id);

    physical_file_location *get_physical_loc_info(file_info *file);
};

typedef struct fsdev::file_system_driver file_device_driver;

#endif