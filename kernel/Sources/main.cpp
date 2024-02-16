#include <kmem_manager.hpp>
#include <debug.hpp>
#include <random.hpp>
#include <interrupt.hpp>
#include <exception.hpp>
#include <segmentation.hpp>
#include <kernel_info.hpp>
#include <io_port.hpp>

#include <drivers/storage_system.hpp>
#include <drivers/block_device_driver.hpp>
#include <drivers/partition_driver.hpp>

#include <integrated/integrated_drivers.hpp>

#include <kernel_argument.hpp>

void pmem_alloc_test(int rand_seed);

extern "C" void kernel_main(unsigned long kernel_info_struct_addr) {
    struct KernelArgument *kargument = (struct KernelArgument *)kernel_info_struct_addr;
    if(kargument->signature != KERNELINFO_STRUCTURE_SIGNATURE) {
        debug::panic("kernel structure not found");
    }

    debug::init();
    debug::out::clear_screen(0x07);
    debug::push_function("kmain");

    memory::pmem_init(kargument->memmap_count , (struct MemoryMap *)((max_t)kargument->memmap_ptr) , kargument);
    set_initial_kernel_info();

    segmentation::init();
    interrupt::init();
    exception::init();
    interrupt::hardware::enable();

    blockdev::init();
    storage_system::init();
    integrated::register_drivers();

    blockdev::block_device *device = blockdev::search_device("idehd" , 0);
    blockdev::block_device_driver *driver = blockdev::BlockDeviceDriverContainer::get_self()->search_by_name("idehd");
    debug::out::printf("driver : %s(0x%X)\n" , driver->driver_name , driver);
    debug::out::printf("device : %s%d(0x%X\n) , " , device->device_driver->driver_name , device->id , device);
    debug::out::printf("block_size   : %d\n" , device->geometry.block_size);
    debug::out::printf("total_count  : %d(=%dB)\n" , device->geometry.lba_total_block_count , device->geometry.lba_total_block_count*device->geometry.block_size);
    storage_system::detect_partitions(device);
    debug::out::printf("device : %s%d\n" , device->device_driver->driver_name , device->id);
    for(int i = 0; i < device->storage_info.logical_block_devs->count; i++) {
        blockdev::block_device *partition_dev = device->storage_info.logical_block_devs->get_object(i);
        debug::out::printf("  partition %d : %ld - %ld\n" , partition_dev->storage_info.partition_id , partition_dev->storage_info.partition_info.physical_sector_start , partition_dev->storage_info.partition_info.physical_sector_end);
    }
    while(1) {
        ;
    }
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