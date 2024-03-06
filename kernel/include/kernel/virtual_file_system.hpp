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
#include <kernel/block_device_driver.hpp>

#include <object_manager.hpp>
#include <hash_table.hpp>
#include <linked_list.hpp>

#define FILE_TYPE_FILE        0x01
#define FILE_TYPE_DIRECTORY   0x02
#define FILE_TYPE_DEVICE_FILE 0x04
#define FILE_TYPE_SYSTEM      0x08
#define FILE_TYPE_READONLY    0x10
#define FILE_TYPE_HIDDEN      0x20

#define FILE_STATUS_NOT_OPEN   0x00
#define FILE_STATUS_OPEN       0x01

#define FILE_NAME_SIZE 256

namespace fsdev { struct file_system_driver; }

struct physical_file_location {
    max_t block_location;
    blockdev::block_device *block_device;
    fsdev::file_system_driver *fs_driver;
};

typedef struct file_info_s {
    char file_name[FILE_NAME_SIZE]; /* Doesn't contain the path, only contains the name of itself */
    word file_type; /* FILE_TYPE_ */
    word file_status; /* FILE_STATUS_ */

    // if file is open
    max_t file_size;
    max_t current_offset; 

    /* For tree structure */
    file_info_s *parent_dir;
    // cached
    ObjectLinkedList<file_info_s> *file_lists; /* only for directory */

    /* physical information */
    bool is_mounted; 
    physical_file_location file_loc_info;
    physical_file_location mount_loc_info;
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
    struct VirtualFileSystemCache { // collections of file_info structures
        SINGLETON_PATTERN_PMEM(VirtualFileSystemCache);

        void init(file_info *rdir , char dir_ident) {
            this->fs_root_dir = rdir;
            fs_root_dir->parent_dir = 0x00;
            fs_root_dir->file_lists = 0x00;
            dir_identifier = dir_ident;
        }
        
        void add_object(file_info *file , file_info *directory) {
            if(directory->file_lists == 0x00) {
                directory->file_lists = new ObjectLinkedList<file_info_s>;
                directory->file_lists->init();
            }
            directory->file_lists->add_object_rear(file);
            file->parent_dir = directory;
        }
        bool remove_object(file_info *file) {
            return false;
        }

        file_info *search_object_last(int level_count , file_info *search_root , char **file_links , int &last_hit_loc) {
            file_info *ptr = search_root;
            int i = 0;
            if(strcmp(file_links[0] , fs_root_dir->file_name) == 0) {
                ptr = fs_root_dir;
                i++;
            }
            for(; i < level_count; i++) {
                last_hit_loc = i;
                if(ptr->file_lists == 0x00) return ptr;
                ObjectLinkedList<file_info_s>::node_s *node = ptr->file_lists->search<char*>([](file_info_s *obj , char *str) { return (strcmp(obj->file_name , str) == 0); } , file_links[i]);
                if(node == 0x00) return ptr;

                ptr = node->object;
            }
            return ptr;
        }
        /// @brief Get how much directory the file is referencing from the file name
        /// @param file_name file name
        /// @return number of directories
        int auto_parse_dir_count(const char *file_name) {
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
        int auto_parse_name(const char *file_name , char **parsed) {    // Auto-allocates
            int i = 0;
            int j = 0;
            int prev_index = 0;
            int dir_count = 0;
            if((dir_count = auto_parse_dir_count(file_name)) != 0) {
                for(i = 0; file_name[i] != 0; i++) {
                    if(file_name[i] == '/') {
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
        file_info *fs_root_dir;
        blockdev::block_device *root_dev;
        
        char dir_identifier;
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
    bool create(const general_file_name file_path);
    file_info *open(const general_file_name file_path , int option);
    bool close(file_info *file);
    bool remove(const general_file_name file_path);

    bool rename(const general_file_name file_path);
    bool move(const general_file_name file_path , const general_file_name new_directory);

    int read(file_info *file , size_t size , void *buffer);
    int write(file_info *file , size_t size , const void *buffer);

    int lseek(file_info *file , max_t cursor , int option);
    
    int read_directory(file_info *file , ObjectLinkedList<char*> &file_list);
}

#endif