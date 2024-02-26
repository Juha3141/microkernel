#include <kmem_manager.hpp>
#include <debug.hpp>
#include <random.hpp>
#include <interrupt.hpp>
#include <exception.hpp>
#include <segmentation.hpp>
#include <kernel_info.hpp>
#include <io_port.hpp>

#include <storage_system.hpp>
#include <block_device_driver.hpp>
#include <char_device_driver.hpp>
#include <partition_driver.hpp>

// #include <integrated/integrated_drivers.hpp>

#include <kernel_argument.hpp>

void pmem_alloc_test(int rand_seed);
void dump_block_devices(void);

extern "C" void kernel_main(unsigned long kernel_argument_struct_addr) {
    struct KernelArgument *kargument = (struct KernelArgument *)kernel_argument_struct_addr;
    if(kargument->signature != KERNELINFO_STRUCTURE_SIGNATURE) {
        debug::panic("kernel structure not found");
    }
    debug::init(kargument);
    debug::out::clear_screen(0x181c24);
    debug::push_function("kmain");

    memory::pmem_init(kargument->memmap_count , (struct MemoryMap *)((max_t)kargument->memmap_ptr) , kargument);
    set_initial_kernel_info();

    segmentation::init();
    interrupt::init();
    exception::init();
    interrupt::hardware::enable();

    blockdev::init();
    chardev::init();
    storage_system::init();
    
    // dump_block_devices();

    while(1) {
        ;
    }
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