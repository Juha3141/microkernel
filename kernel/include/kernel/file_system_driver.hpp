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
    // 
    struct file_system_driver {
        virtual bool check(blockdev::block_device *device);

        virtual bool create(file_info *new_file , file_info *directory);

        // fill out the structure
        virtual bool open(file_info *file , int option);
        virtual bool close(file_info *file);
        virtual bool remove(file_info *file);

        virtual bool rename(file_info *new_file);
        virtual bool move(file_info *new_file , file_info *new_directory);

        virtual int read(file_info *file , size_t size , void *buffer);
        virtual int write(file_info *file , size_t size , const void *buffer);

        virtual int lseek(file_info *file , max_t cursor , int option);

        virtual int read_directory(file_info *file , max_t cursor);

        char fs_string[32];
    };

    class FileSystemDriverManager : public ObjectManager<file_system_driver> {
        SINGLETON_PATTERN_PMEM(FileSystemDriverManager);
        
        bool check(max_t id , blockdev::block_device *device) {
            if(!object_container[id].occupied) return false;
            if(id >= max_count) return false;
            return object_container[id].object->check(device);
        }

        blockdev::block_device *head_device; // The root device 
    };
    void init(void);

    max_t register_driver(file_system_driver *driver , const char *fs_name);
    file_system_driver *search_driver(const char *fs_name);
    file_system_driver *search_driver(max_t driver_id);

    max_t discard_driver(const char *fs_name);
    max_t discard_driver(max_t driver_id);

    void set_head_device(blockdev::block_device *head_dev);
};

typedef struct fsdev::file_system_driver file_device_driver;

#endif