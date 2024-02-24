#ifndef _BLOCK_DEVICE_DRIVER_HPP_
#define _BLOCK_DEVICE_DRIVER_HPP_

#include <device_driver.hpp>
#include <block_device_scheduler.hpp>

namespace blockdev {
    // pre
    struct block_device_driver;
    class BlockDeviceContainer;

    typedef struct partition_info_s {
        max_t physical_sector_start;
        max_t physical_sector_end;

        bool bootable;
    }partition_info_t;

    typedef enum storage_type_s {
        physical=0 , logical=1
    }storage_type_t;

    struct device_geometry {
        max_t lba_total_block_count;
        max_t block_size;

        bool is_chs;
        max_t cylinder;
        max_t head;
        max_t sector;
    };

    struct storage_info_t {
        max_t partition_driver_id;
        storage_type_t storage_type;
        
        /* For logical storage */
        max_t partition_id;
        // If physical_sector_start == 0 --> Physical disk, "physical_sector_end" describes the total sector count
        // If physical_sector_start != 0 --> Logical disk
        partition_info_t partition_info; 

        /* Common */
        blockdev::BlockDeviceContainer *logical_block_devs;
    };
    struct block_device : general_device<block_device_driver> {

        // Geometry information
        device_geometry geometry;
        // Storage information
        storage_info_t storage_info;

        max_t mount_id;
        // FileSystemDriver *fs_driver;
    };
    struct block_device_driver : general_device_driver<BlockDeviceContainer> {
        virtual bool prepare(void) = 0;
        virtual max_t read(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual max_t write(block_device *device , max_t block_address , max_t count , void *buffer) = 0;
        virtual bool get_geometry(block_device *device , device_geometry &geometry) = 0;
        virtual bool io_read(block_device *device , max_t command , max_t argument , max_t &data_out) = 0;
        virtual bool io_write(block_device *device , max_t command , max_t argument) = 0;

        bool use_auto_partition_detector;

        bdevsched::block_io_scheduler *scheduler;
        bdevsched::block_io_queue *io_queue;
    };
    
    class BlockDeviceContainer : public ObjectManager<block_device> {};
    struct BlockDeviceDriverContainer : public ObjectManager<block_device_driver> {
        SINGLETON_PATTERN_PMEM(BlockDeviceDriverContainer);
    };
    void init(void);

    max_t register_driver(block_device_driver *driver , const char *driver_name);
    block_device_driver *search_driver(const char *driver_name);
    block_device_driver *search_driver(max_t driver_id);

    max_t discard_driver(const char *driver_name);
    max_t discard_driver(max_t driver_id);
    
    max_t register_device(block_device_driver *driver , block_device *device);
    max_t register_device(const char *driver_name , block_device *device);
    max_t register_device(max_t driver_id , block_device *device);
    block_device *search_device(block_device_driver *driver , max_t device_id);
    block_device *search_device(const char *driver_name , max_t device_id);
    block_device *search_device(max_t driver_id , max_t device_id);

    bool discard_device(block_device *device);

    block_device *create_empty_device(void);
    void designate_resources_count(block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count);
};

#endif