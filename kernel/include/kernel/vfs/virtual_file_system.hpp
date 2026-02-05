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

#include <kernel/essentials.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/driver/char_device_driver.hpp>

#include <object_manager.hpp>
#include <hash_table.hpp>
#include <linked_list.hpp>
#include <queue.hpp>

#define FILE_TYPE_FILE        0x01
#define FILE_TYPE_DIRECTORY   0x02
#define FILE_TYPE_DEVICE_FILE 0x04
#define FILE_TYPE_SYSTEM      0x08
#define FILE_TYPE_READONLY    0x10
#define FILE_TYPE_HIDDEN      0x20

#define FILE_NAME_SIZE 256

namespace fsdev { struct file_system_driver; }

struct physical_file_location {
    max_t block_location;
    blockdev::block_device *block_device;
    fsdev::file_system_driver *fs_driver;
};

// block_cache_t : Cache structure of one block, contains cached block data and some other informations.
typedef struct block_cache_s {
    // true  : the contents of the cache is the same as the contents in the disk
    // false : the contents of the cache is different with the contents in the disk
    bool flushed;

    void *block;
    max_t block_size;
    // for newly created caches
    max_t linear_block_addr;
}block_cache_t;

typedef struct open_info_s {
    max_t task_id;
    max_t open_flag;

    max_t maximum_offset;
    max_t open_offset;

    // Hash table for file buffer
    // Key value : Physical block location
    HashTable<block_cache_t , max_t> *cache_hash_table;

    // List for newly created blocks
    // Key value : Logical block location
    LinkedList<block_cache_t*> *new_cache_linked_list;
}open_info_t;

typedef struct file_info_s {
    char file_name[FILE_NAME_SIZE]; /* Doesn't contain the path, only contains the name of itself */
    word file_type; /* FILE_TYPE_ */
    max_t file_size;

    /* For tree structure */
    file_info_s *parent_dir;
    // cached
    LinkedList<file_info_s*> *file_list; /* only for directory */
    
    /* physical information */
    bool is_mounted; 
    physical_file_location file_loc_info;
    physical_file_location mount_loc_info;

    LinkedList<open_info_t*> *who_open_list;
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
    file_info *file;
    
}directory_cache_info;

namespace vfs {
    struct DirectoryCacheManager : HashTable<directory_cache_info , char*> {
        SINGLETON_PATTERN_PMEM(DirectoryCacheManager);
    };
    struct VirtualFileSystemManager { // General VFS manager
        SINGLETON_PATTERN_PMEM(VirtualFileSystemManager);

        void init(file_info *rdir , blockdev::block_device *root_device , char dir_ident);
        void add_object(file_info *file , file_info *directory);
        bool remove_object(const char *file_name , file_info *directory);

        file_info *search_object_last(int level_count , file_info *search_root , char **file_links , int &last_hit_loc);
        int auto_parse_dir_count(const char *file_name);
        int auto_parse_name(const char *file_name , char **parsed);

        void get_file_base_name(const char *full_file_path , char *base_name);

        // root directory
        file_info *fs_root_dir;
        // root device
        blockdev::block_device *root_dev;
        // directory identifier, default '/'
        char dir_identifier;

        // accuracy sake
        bool is_initialized_properly; 
    };

    void init(blockdev::block_device *root_device); 

    file_info *get_root_directory(void);

    file_info *create_file_info_struct(
        const physical_file_location file_loc ,
        const char *file_name ,
        int file_type ,
        int file_size);

    bool mount(file_info *file , blockdev::block_device *device);
    bool unmount(file_info *file , blockdev::block_device *device);

    // general function for general purpose
    bool create(const general_file_name file_path , word file_type);
    file_info *open(const general_file_name file_path , int option);
    bool flush(file_info *file);
    bool close(file_info *file);
    bool remove(const general_file_name file_path);

    bool rename(const general_file_name file_path , const char *new_name);
    bool move(const general_file_name file_path , const general_file_name new_directory);

    long read(file_info *file , max_t size , void *buffer);
    long write(file_info *file , max_t size , const void *buffer);

    long lseek(file_info *file , long cursor , int option);
    
    int read_directory(file_info *file);
}

#endif