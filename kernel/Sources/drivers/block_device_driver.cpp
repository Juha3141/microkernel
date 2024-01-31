/**
 * @file block_device_driver.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Kernel block device driver
 * @date 2024-01-22 (Original version : https://github.com/Juha3141/OS, StorageDriver.cpp)
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */

#include <drivers/block_device_driver.hpp>
#include <drivers/partition_driver.hpp>
#include <linked_list.hpp>

#include <kmem_manager.hpp>

#include <debug.hpp>

void blockdev::init(void) {
    // BlockDeviceDriverContainer : global container for kernel
    BlockDeviceDriverContainer::get_self()->init(512);
    PartitionDriverContainer::get_self()->init(64);
}

/// @brief Register the block device driver
/// @param driver Driver structure
/// @param driver_name Name of the driver
/// @return Return the id of driver
max_t blockdev::register_driver(blockdev::block_device_driver *driver , const char *driver_name) {
    debug::push_function("bdev::reg_drv");

    BlockDeviceDriverContainer *driver_container = BlockDeviceDriverContainer::get_self();
    max_t id = driver_container->register_driver(driver); // register driver to global container
    if(id == INVALID) { debug::pop_function(); return INVALID; }
    
    // assign new local device container
    // Driver contains its devices
    driver->device_container = new class BlockDeviceContainer;
    driver->device_container->init(256); // Initialize the device container
    strcpy(driver->driver_name , driver_name);
    driver->prepare();
    
    debug::out::printf("Registered device driver, id : %d name : \"%s\"\n" , driver->driver_id , driver->driver_name);
    debug::pop_function();
    return driver->driver_id;
}

///@brief some bridge-like functions(just basic stuff)

blockdev::block_device_driver *blockdev::search_driver(const char *driver_name) { return BlockDeviceDriverContainer::get_self()->search_by_name(driver_name); }
blockdev::block_device_driver *blockdev::search_driver(max_t driver_id) { return BlockDeviceDriverContainer::get_self()->get_object(driver_id); }

max_t blockdev::discard_driver(const char *driver_name) { return BlockDeviceDriverContainer::get_self()->discard_object(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name)); }
max_t blockdev::discard_driver(max_t driver_id) { return BlockDeviceDriverContainer::get_self()->discard_object(BlockDeviceDriverContainer::get_self()->get_object(driver_id)); }

static bool set_physical_block_device_info(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    device->logical_devices = 0x00;
    memset(&device->logical_partition_info , 0 , sizeof(blockdev::partition_info));
    if(driver->get_device_geometry(device , &device->geomtery) == false) {
        return false;
    }
    device->id = driver->device_container->register_object(device);
    if(device->id == INVALID) {
        return false;
    }
    device->device_driver = driver;
    device->partition_id = INVALID;
    return true;
}

static void set_logical_block_device_info(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    device->device_driver = new class blockdev::logical_block_device_driver;
    // polymorphism!
    ((blockdev::logical_block_device_driver *)device->device_driver)->set_super_driver(driver);
}

/// @brief Registeres device to driver(kernel), identifies/registers partition and designates file system driver for device
/// @param driver Target driver
/// @param device Device to be registered
/// @return Return the id of the device
max_t blockdev::register_device(blockdev::block_device_driver *driver , blockdev::block_device *device) {
    debug::push_function("blockdev::reg_dev");
    BlockDeviceDriverContainer *driver_container = BlockDeviceDriverContainer::get_self();
    
    int partition_count;
    DataLinkedList<blockdev::partition_info>partition_info_list;
    PartitionDriver *partition_driver;
    if(driver == 0x00) return INVALID;
    if(device == 0x00) return INVALID;
    // Initialize & write necessary information to the device
    // The function set_physical_block_device_info sets the basic information of the device
    // and calls get_device_geometry from the driver

    // The function set_logical_block_Device_info creates new device driver for logical device.
    // Upon creating new logical block device driver, the function sets the super driver
    // to the physical driver.

    // What logical_block_device_driver does is interpreting the logical sector value to physical sector info.
    
    /* To-do : Further information & maintenance for logical block devices are required! */
    if(device->storage_type == physical) {
        if(set_physical_block_device_info(driver , device) == false) return false;
        debug::out::printf("Registered physical block device driver : #%d(%s), driver %s\n" , device->id , (device->storage_type == physical) ? "physical" : "logical" , driver->driver_name);
    }
    if(device->storage_type == logical) {
        set_logical_block_device_info(driver , device);
    }
    
    // add necessary logical block devices

    // The logical partitions are formed as "some kind of" tree structure..
    // Say -- One device contains the object container containing one logical device,,
    // and that logical device contains the object container and so on,,,
    max_t partitiondrv_id = identify_partition_driver(device);
    partition_driver = get_partition_identifier(device);
    if(partition_driver != 0x00) {
        partition_count = partition_driver->get_partitions_list(device , partition_info_list);

        device->logical_devices = new BlockDeviceContainer;
        device->logical_devices->init(72); // maximum logical storages count

        add_logical_device(driver , device , partition_info_list);
    }
    // To-do : Add file system driver stuff!

    return device->id;
}

/// @brief Other form of register_device

max_t blockdev::register_device(const char *driver_name , blockdev::block_device *device) { return blockdev::register_device(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name) , device); }
max_t blockdev::register_device(max_t driver_id , blockdev::block_device *device) { return blockdev::register_device(BlockDeviceDriverContainer::get_self()->get_object(driver_id) , device); }

blockdev::block_device *blockdev::search_device(max_t driver_id , max_t device_id , max_t partition_id) {
    blockdev::block_device_driver *driver = search_driver(driver_id);
    blockdev::block_device *device = driver->device_container->get_object(device_id);
    if(partition_id == INVALID) return device;
    return device->logical_devices->get_object(partition_id);
}

blockdev::block_device *blockdev::search_device(blockdev::block_device_driver *driver , max_t device_id , max_t partition_id) {  return search_device(driver->driver_id , device_id , partition_id); }
blockdev::block_device *blockdev::search_device(const char *driver_name , max_t device_id , max_t partition_id) { return blockdev::search_device(BlockDeviceDriverContainer::get_self()->search_by_name(driver_name) , device_id , partition_id); }

bool blockdev::discard_device(blockdev::block_device *device);

/// @brief Create empty device with essential informations
/// @param driver driver for device
/// @param storage_type type of storage
/// @return new empty device
blockdev::block_device *blockdev::create_empty_device(blockdev::block_device_driver *driver , storage_type_t storage_type) {
    block_device *device = (block_device *)memory::pmem_alloc(sizeof(block_device));
    device->device_driver = driver;
    device->storage_type = storage_type;
    return device;
}

template <typename T> inline T *alloc_by_cnt(int cnt) { return (T *)memory::pmem_alloc(cnt*sizeof(T)); } 

void blockdev::designate_resources_count(blockdev::block_device *device , int io_port_count , int interrupt_count , int flags_count , int etc_res_count) {
    if(io_port_count > 0) {
        device->resource.io_port_count = io_port_count;
        device->resource.io_ports = alloc_by_cnt<io_port>(io_port_count);
    }
    if(flags_count > 0) {
        device->resource.flags_count = flags_count;
        device->resource.flags = alloc_by_cnt<resource_flag_t>(flags_count);
    }
    if(etc_res_count > 0) {
        device->resource.etc_resources_count = etc_res_count;
        device->resource.etc_resources = alloc_by_cnt<etc_resource_t>(etc_res_count);
    }
}

/* To-do : The function add_logical_device is very ambiguous!!
 * We need to make the replacement of function.
 */

/// @brief Add logical drive to Storage (Disclaimer : This function doesn't calculate how full the storage is)
//         Use at your own risk, this function can't guarantee the stability when disk's full.
/// @param driver driver (= device->device_driver)
/// @param device Target device
/// @param partitions list of partitions
void blockdev::add_logical_device(blockdev::block_device_driver *driver , blockdev::block_device *device , DataLinkedList<blockdev::partition_info> &partitions) {
    int j = 0;
    int current_partition_count;
    block_device *logical_device;
    if(device->logical_devices == 0x00) {
        device->logical_devices = new BlockDeviceContainer;
        device->logical_devices->init(72);
    }
    current_partition_count = device->logical_devices->count;
    
    DataLinkedList<blockdev::partition_info>::node_s *ptr = partitions.get_start_node();

    while(ptr != 0x00) {
        logical_device = create_empty_device(driver , logical);
        // copy resources info
        memcpy(&logical_device->resource , &device->resource , sizeof(device_resources));
        // Write info of parent 
        logical_device->id = device->id;
        logical_device->storage_type = logical;
        memcpy(&logical_device->geomtery , &device->geomtery , sizeof(device_geometry)); 
        memcpy(&logical_device->logical_partition_info , &device->logical_partition_info , sizeof(partition_info));

        // to-do : fix infinite loop error
        logical_device->partition_id = device->logical_devices->register_object(logical_device);
        blockdev::register_device(driver , logical_device);

        ptr = ptr->next;
    }
}

/// @brief get the physical super device of the logical block device
/// @param device the logical block device
/// @return the physical super block device
blockdev::block_device *blockdev::get_physical_super_device(blockdev::block_device *logical_device) {
    if(logical_device->storage_type != logical) return 0x00;
    logical_block_device_driver *driver = (logical_block_device_driver *)logical_device->device_driver;
    if(driver == 0x00) return 0x00;
    if(driver->super_driver == 0x00) return 0x00;
    return driver->super_driver->device_container->get_object(logical_device->id);
}