/**
 * @file virtual_file_system.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The kernel virtual file system. Caches & translates(if necessary) the file operations. 
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho.
 * 
 */

#ifndef _VFS_HPP_
#define _VFS_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/file_system_driver.hpp>
#include <kernel/block_device_driver.hpp>

#include <object_manager.hpp>
#include <linked_list.hpp>

#define FILE_TYPE_FILE        1
#define FILE_TYPE_DIRECTORY   2
#define FILE_TYPE_DEVICE_FILE 3
#define FILE_TYPE_SYSTEM      4

#define FILE_STATUS_NOT_OPEN   0x01
#define FILE_STATUS_READ       0x02
#define FILE_STATUS_WRITE      0x04
#define FILE_STATUS_BLANKOUT   0x08

typedef struct file_info_s {
    char *file_name; /* Doesn't contain the path, only contains the name of itself */
    max_t file_type; /* FILE_TYPE_ */
    max_t file_status; /* FILE_STATUS_ */
    max_t file_size;
    max_t current_offset; 

    max_t block_location;

    // cached
    file_info_s *previous_file;
    file_info_s **file_lists; /* only for directory */

    // physical side of file
    blockdev::block_device *block_device;
    fsdev::file_system_driver *fs_driver;

    // mounted location
    file_info_s *mounted_loc;
}file_info;

typedef struct general_file_name_s {
    // full file path
    char *file_name;

    // root directory
    file_info *root_directory;
}general_file_name;

typedef struct directory_cache_info_s {
    // full directory name that will be cached
    char *full_directory_name;
    
    // pointer to the file info
    file_info *file_info;
    
}directory_cache_info;

namespace vfs {
    class DirectoryCacheManager { // Hash Table
        public:
            SINGLETON_PATTERN_PMEM(DirectoryCacheManager);
            
            
    };
    class VirtualFileSystemCache { // collections of file_info structures
        public:
            SINGLETON_PATTERN_PMEM(VirtualFileSystemCache);

            
    };
    // general function for general purpose
    bool create(const general_file_name file_path);
    file_info *open(const general_file_name file_path , int option);
    bool close(file_info *file);
    bool remove(const general_file_name file_path);

    bool rename(const general_file_name file_path);
    bool move(const general_file_name file_path , const general_file_name new_directory);

    int read(file_info *file , size_t size , void *buffer);
    int write(file_info *file , size_t size , const void *buffer);

    int lseek(file_info *file , max_t cursor , int option);
    
    int read_directory(file_info *file , ObjectLinkedList<char*>file_list);
}

#endif