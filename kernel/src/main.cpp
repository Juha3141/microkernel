#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/driver/block_device_driver.hpp>
#include <kernel/driver/char_device_driver.hpp>

#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/partition_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>

#include <kernel/kernel_argument.hpp>

#include <kernel/integrated.hpp>

#include <integrated_drivers.hpp>
#include <file_systems.hpp>

#include <arch_inline_asm.hpp>

// For testing

#include <random.hpp>

#include <hash_table.hpp>

void pmem_alloc_test(int rand_seed);
void dump_block_devices(void);
void test_hash(void);

void demo_routine(void);

extern "C" __attribute__ ((section(".entry"))) void kernel_main(unsigned long kernel_argument_struct_addr) {
    struct KernelArgument *kargument = (struct KernelArgument *)kernel_argument_struct_addr;
    if(kargument->signature != KERNELINFO_STRUCTURE_SIGNATURE) {
        debug::panic("kernel structure not found");
    }
    debug::init(kargument);
    debug::out::clear_screen(0x07);
    memory::pmem_init(kargument->memmap_count , (struct MemoryMap *)((max_t)kargument->memmap_ptr) , kargument);

    segmentation::init();
    interrupt::init();
    exception::init();
    interrupt::hardware::enable();

    debug::out::printf(DEBUG_INFO , "----- Initializing block device driver..\n");
    blockdev::init();
    storage_system::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing file system driver..\n");
    fsdev::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing character device driver..\n");
    chardev::init();
    register_basic_kernel_drivers();

    interrupt::hardware::enable();

    while(1) {
        ;
    }

    debug::out::printf(DEBUG_INFO , "----- Initializing vfs..\n");
    debug::out::printf(DEBUG_INFO , "Setting root directory to the provided ramdisk : 0x%lx-0x%lx\n" , kargument->ramdisk_address , kargument->ramdisk_address+kargument->ramdisk_size);
    // find the ramdisk driver
    if(kargument->ramdisk_address == 0x00) {
        debug::panic("No ramdisk found!\n"); // frick! I forgot to add the ramdisk!
    }
    blockdev::block_device *device = ramdisk_driver::create(kargument->ramdisk_size/512 , 512 , kargument->ramdisk_address);
    blockdev::register_device(device->device_driver->driver_id , device);
    
    // mount to the root device
    vfs::init(device);
    debug::out::printf("memory usage : %dKB\n" , memory::pmem_usage()/1024);

//  demo_routine();
//  dump_block_devices();

    while(1) {
        ;
    }
}

void demo_routine(void) {
    file_info *rootdir = vfs::get_root_directory();
    int file_count = vfs::read_directory(rootdir);
    debug::out::printf(DEBUG_INFO , "Number of files in the root directory : %d\n" , file_count);
    ObjectLinkedList<file_info_s>::node_s *f_ptr_node = rootdir->file_list->get_start_node();
    for(int i = 0; i < file_count; i++) {
        if(f_ptr_node == 0x00) break;
        if(f_ptr_node->object->file_type == FILE_TYPE_DIRECTORY) debug::out::printf("%s [DIRECTORY]\n" , f_ptr_node->object->file_name);
        else debug::out::printf("%s, size = %d\n" , f_ptr_node->object->file_name , f_ptr_node->object->file_size);
        f_ptr_node = f_ptr_node->next;
    }

    // read file Hello.txt
    vfs::create({"hello-new.txt" , rootdir} , FILE_TYPE_FILE);
    file_info *new_file = vfs::open({"hello-new.txt" , rootdir} , FILE_OPEN_RW);
    vfs::write(new_file , 25 , "This is a brand new file!");
    
    vfs::lseek(new_file , 0 , LSEEK_SET);
    char buffer[25] = {0 , };
    vfs::read(new_file , 25 , buffer);
    
    debug::out::printf("buffer : %s\n" , buffer);
}

void dump_block_devices(void) {
    debug::push_function("dump_bd");
    for(int a=0;a<79;a++) debug::out::raw_printf("-");
    debug::out::raw_printf("\n");
    blockdev::BlockDeviceDriverContainer *blockdev_container = blockdev::BlockDeviceDriverContainer::get_self();
    debug::out::printf(DEBUG_SPECIAL , "Dumping the block device tree...\n");
    for(int i = 0; i < blockdev_container->max_count; i++) {
        blockdev::block_device_driver *driver = blockdev_container->get_object(i);
        if(driver == 0x00) continue;
        debug::out::printf(DEBUG_INFO , "%02d. Registered driver %s ----- \n" , i , driver->driver_name);
        if(driver->device_container->count == 0) {
            debug::out::printf("    no device found.\n");
            continue;
        }
        for(int j = 0; j < driver->device_container->max_count; j++) {
            blockdev::block_device *device = driver->device_container->get_object(j);
            if(device == 0x00) continue;

            debug::out::printf(DEBUG_INFO , "      device %d (%s%d), capacity : %d sectors( = %dMB)\n" , device->id , driver->driver_name , device->id , device->geometry.lba_total_block_count , device->geometry.block_size*device->geometry.lba_total_block_count/1000/1000);
            if(device->storage_info.logical_block_devs == 0x00) {
                debug::out::printf("      partition count : 0\n");
                continue;   
            };
            debug::out::printf("      partition count : %d\n" , device->storage_info.logical_block_devs->count);
            if(device->storage_info.logical_block_devs->count == 0) continue; 
            blockdev::BlockDeviceContainer *logical_devs = device->storage_info.logical_block_devs;

            for(int k = 0; k < logical_devs->max_count; k++) {
                blockdev::block_device *device = logical_devs->get_object(k);
                if(device == 0x00) continue;
                debug::out::printf("         partition #%d : %ld - %ld\n" , device->storage_info.partition_id , device->storage_info.partition_info.physical_sector_start , device->storage_info.partition_info.physical_sector_end);
            }
        }
    }
    debug::out::printf(DEBUG_SPECIAL , "Done probing the block device tree!\n");
    for(int a=0;a<79;a++) debug::out::raw_printf("-");
    debug::out::raw_printf("\n");
    debug::pop_function();
}

void pmem_alloc_test(int rand_seed) {
    debug::push_function("pmem_a_t");
    debug::out::printf("Physical memory allocation test\n");
    max_t alloc_size[100] = {0 , };
    debug::out::printf("current usage : %d\n" , memory::pmem_usage());
    max_t ptr_list[100] = {0 , };
    dword demanded = 0;
    for(int i = 0; i < 8; i++) {
        alloc_size[i] = (rand()+512)%4096;
        max_t ptr = (max_t)memory::pmem_alloc(alloc_size[i]);
        memset((vptr_t *)ptr , 0xDA , alloc_size[i]);
        debug::out::printf("%d : 0x%X - 0x%X\n" , i , ptr , ptr+alloc_size[i]);
        ptr_list[i] = ptr;
        demanded += alloc_size[i];
    }
    debug::out::printf("physical usage       : %d\n" , memory::pmem_usage());
    debug::out::printf("allocation requested : %d\n" , demanded);
    for(int i = 0; i < 8; i++) {
        memset((vptr_t *)ptr_list[i] , 0 , alloc_size[i]);
        memory::pmem_free((vptr_t *)ptr_list[i]);
        debug::out::printf("%d : Free 0x%X - usage %d\n" , i , ptr_list[i] , memory::pmem_usage());
    }
    debug::pop_function();
}