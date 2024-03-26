#include <kernel/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt.hpp>
#include <kernel/exception.hpp>
#include <kernel/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/storage_system.hpp>
#include <kernel/block_device_driver.hpp>
#include <kernel/char_device_driver.hpp>
#include <kernel/partition_driver.hpp>
#include <kernel/virtual_file_system.hpp>

#include <kernel/kernel_argument.hpp>

#include <kernel/integrated.hpp>

#include <integrated_drivers.hpp>
#include <file_systems.hpp>

// For testing

#include <random.hpp>

#include <hash_table.hpp>

void pmem_alloc_test(int rand_seed);
void dump_block_devices(void);
void test_hash(void);

extern "C" void kernel_main(unsigned long kernel_argument_struct_addr) {
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
    char buffer[128];
    char buffer2[512];

    file_info *file_hello = vfs::open({"@/Hello.txt" , 0x00} , FILE_OPEN_RW);
    vfs::create({"@/test_disk" , 0x00} , FILE_TYPE_DIRECTORY);
    file_info *mount = vfs::open({"@/test_disk" , 0x00} , FILE_OPEN_RW);
    vfs::mount(mount , device);
    int rd = vfs::read(file_hello , 128 , buffer);
    // debug::dump_memory((max_t)buffer , 128 , true);
    debug::out::printf("rd = %d\n" , rd);
    rd = vfs::read(file_hello , 128 , buffer);
    // debug::dump_memory((max_t)buffer , 128 , true);
    
    debug::out::printf("rd = %d\n" , rd);

    rd = vfs::read(file_hello , 512 , buffer2);
    debug::out::printf("rd = %d\n" , rd);
    // debug::dump_memory((max_t)buffer2 , 512 , true);
    debug::out::printf("--------- write ---------\n");

    vfs::write(file_hello , 14 , "Hello, world!");

    vfs::lseek(file_hello , -14 , LSEEK_CUR);
    debug::out::printf("--------- read ---------\n");
    char buffer3[20] = {0 , };
    vfs::read(file_hello , 14 , buffer3);
    debug::out::printf("%s\n" , buffer3);

    debug::out::printf("--------- write ---------\n");
    vfs::lseek(file_hello , 0 , LSEEK_END);
    vfs::write(file_hello , 28 , "testing at the rear of file!");

    char buffer4[100] = {0 , };
    vfs::lseek(file_hello , -100 , LSEEK_CUR);
    rd = vfs::read(file_hello , 100 , buffer4);
    debug::dump_memory((max_t)buffer4 , rd , true);
    
    while(1) {
        ;
    }
}

struct my_hash_obj_s {
    int a;
    int b;
};

void test_hash(void) {
    HashTable<struct my_hash_obj_s , char*> test_hash;
    struct my_hash_obj_s obj = {12 , 34};
    test_hash.init(512 , 
    [](char *(&dest) , char *src){ dest = (char*)memory::pmem_alloc(strlen(src)+1); memcpy(dest , src , strlen(src)); dest[strlen(src)] = 0; } , 
    [](char *dest , char *src) { return (bool)(strcmp(dest , src) == 0); } , hash_function_string);
    
    test_hash.add("hello" , &obj);
    debug::out::printf("hash \"hello\" : %d, obj : 0x%X\n" , test_hash.hash_function("hello") , &obj);
    test_hash.add("world" , &obj);
    debug::out::printf("hash \"world\" : %d, obj : 0x%X\n" , test_hash.hash_function("world") , &obj);
    struct my_hash_obj_s *res = test_hash.search("hello");
    debug::out::printf("res : 0x%X\n" , res);
    debug::out::printf("res.a = %d\n" , res->a);
    debug::out::printf("res.b = %d\n" , res->b);
    res = test_hash.search("world");
    debug::out::printf("res : 0x%X\n" , res);
    debug::out::printf("res.a = %d\n" , res->a);
    debug::out::printf("res.b = %d\n" , res->b);
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