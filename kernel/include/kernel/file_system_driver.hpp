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
    struct file_system_driver {
        // check whether the device has file system
        virtual bool check(blockdev::block_device *device) = 0;
        // Get the physical location of root directory
        virtual bool get_root_directory(physical_file_location &file_loc) = 0;

        // Create new file
        virtual bool create(const general_file_name file_name , word file_type) = 0;

        // Get file handle by file name
        virtual file_info *get_file_handle(const general_file_name file_name) = 0;

        // remove the file by file name
        virtual bool remove(const general_file_name file_name) = 0;

        // Rename the file
        virtual bool rename(const general_file_name file_name , const char *new_file_name) = 0;
        // Move the file
        virtual bool move(const general_file_name file_name , file_info *new_directory) = 0;

        // Read a block of the file
        virtual bool read_block(file_info *file , max_t file_block_addr , void *buffer) = 0;
        // Write to a block of file
        virtual bool write_block(file_info *file , max_t file_block_addr , const void *buffer) = 0;
        // Allocate a new block to file
        // return : "Sector address"
        // unit_size : How many "Sector" is one block allocated by fsdev?
        virtual max_t allocate_new_block(file_info *file , max_t &unit_size) = 0;

        // Get physical block address of the file
        virtual max_t get_phys_block_address(file_info *file , max_t linear_block_addr) = 0;

        // Read the directory
        virtual int read_directory(file_info *file , max_t cursor) = 0;

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