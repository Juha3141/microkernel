#ifndef _FILE_SYSTEM_DRIVER_HPP_
#define _FILE_SYSTEM_DRIVER_HPP_

#include <kernel/driver/device_driver.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>
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

        /*
         * "Cluster" here indicates the group of linear blocks that forms one unit of allocation for new space. 
         * (It is the identical concept of cluster from the FAT file system.)
         */

        /// @brief Translate linear block address into the start address of the corresponding cluster.
        /// @param file file
        /// @param linear_block_addr Linear block address of file
        /// @return Sector address of the cluster
        virtual max_t get_cluster_start_address(file_info *file , max_t linear_block_addr) = 0;

        /// @brief Get the number of sector that consists one cluster
        /// @param file file
        /// @return Number of sector of a cluster
        virtual max_t get_cluster_size(file_info *file) = 0;
        
        /// @brief Allocate a new blocks to file (allocate one cluster to the file)
        /// @param file file
        /// @return Start address of the cluster 
        virtual max_t allocate_new_cluster_to_file(file_info *file) = 0;

        /// @brief Read the directory, save the file_info structure to the linked list
        /// ***Note : This function must create a new file_info structure and store to the linked list. 
        ///           Allocate the new file_info object (for each file) using pmem allocator and 
        ///           store it to the linked list. 
        /// @param file the directory to read
        /// @param file_list Where the file_info structures are stored
        /// @return Number of files
        virtual int read_directory(file_info *file , LinkedList<file_info*> &file_list) = 0;

        /// @brief Apply the change of file_info. Note that this function Cannot change the file name. 
        ///        This function can only change the file size or other things. 
        /// @param file file
        /// @param new_size new file size
        /// @return True if succeed
        virtual bool apply_new_file_info(file_info *file , max_t new_size/* , to-do : date*/) = 0;

        char fs_string[32];
    };

    struct FileSystemDriverContainer : public FixedArray<file_system_driver*> {
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