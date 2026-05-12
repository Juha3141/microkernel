#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kasan.hpp>

#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/io_port.hpp>
#include <kernel/driver/device_driver.hpp>
#include <kernel/driver/general_input_system.hpp>
#include <kernel/driver/pci.hpp>

#include <ramdisk/ramdisk.hpp>
#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>

#include <kernel/debug.hpp>

#include <kernel/sections.hpp>
#include <loader/loader_argument.hpp>

#include <kernel/init.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

extern "C" void sanitized_kernel_main(LoaderArgument *loader_argument);

__no_sanitize_address__
__entry_function__
extern "C" void kernel_main(LoaderArgument *loader_argument , max_t kernel_vmem_addr , max_t kernel_stack_vmem_addr , max_t kernel_stack_size , max_t pt_space_phys_start , max_t pt_space_phys_end) {
    memory::kstruct_init(loader_argument);
    memory::kmemmap_init(loader_argument);
    /* Set up the virtual memory for KASan shadow memory space
     * Using the map_usable_mem_into_vaddr() function, utilize the segmented free memory space and map them into 
     * one contiguous virtual memory space that will be used for shadow mem.

     * The memory space will be located at CONFIG_KERNEL_KASAN_VMA. The actual shadow memory starts at the address
     * CONFIG_KERNEL_KASAN_VMA + CONFIG_KERNEL_VMA >> KASAN_SHADOW_SHIFT
     */
#ifdef CONFIG_USE_KASAN
    Boundary kasan_vmem_boundary = setup_kasan_shadowmem(loader_argument->kernel_size , kernel_stack_size);
    max_t kasan_vmem_size = kasan_vmem_boundary.end_address - kasan_vmem_boundary.start_address;

    kasan::init(kasan_vmem_size);
#endif

    debug::init(loader_argument);
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the higher-half kernel!\n");

    debug::out::printf("========================== Kernel memory map ==========================\n");
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();
    while(kmemmap_ptr != nullptr) {
        debug::out::printf("0x%-16llx ~ 0x%-16llx % 13lldkB (%s)\n" , kmemmap_ptr->start_address , kmemmap_ptr->end_address , (kmemmap_ptr->end_address-kmemmap_ptr->start_address)/1024 , memory::memmap_type_to_str(kmemmap_ptr->type));
        kmemmap_ptr = kmemmap_ptr->next;
    }
    memory::pmem_init();
    sanitized_kernel_main(loader_argument);
    while(1) {
        ;
    }
}

block_device *find_root_block_device(block_device *ramdisk);

__entry_function__
extern "C" void sanitized_kernel_main(LoaderArgument *loader_argument) {
    debug::out::printf(DEBUG_INFO , "----- Initializing segmentation system..\n");
    segmentation::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing interrupt system..\n");
    interrupt::init();
    exception::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing general device driver system..\n");
    dev::init();
    storage_system::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing file system driver..\n");
    fsdev::init();
    input::init();
    pci::probe_all_pci_devices();
    dev::register_file_system_drivers();
    dev::register_kernel_drivers();

    interrupt::hardware::enable();

    debug::out::printf(DEBUG_INFO , "----- Initializing vfs..\n");
    debug::out::printf(DEBUG_INFO , "Setting root directory to the provided ramdisk : 0x%lx-0x%lx\n" , loader_argument->ramdisk_location , loader_argument->ramdisk_location+loader_argument->ramdisk_size);
    
    // find the ramdisk driver
    block_device *ramdisk = nullptr;
    if(loader_argument->is_ramdisk_available) {
        ramdisk = ramdisk_driver::create(loader_argument->ramdisk_size/512 , 512 , loader_argument->ramdisk_location);
    }
    // Check ramdisk, if partition exists
    block_device *root_device = find_root_block_device(ramdisk);
    if(root_device == nullptr) {
        debug::panic("Unable to find any boot device!!\n");
    }
    vfs::init(root_device);

    file_info *root_dir = vfs::get_root_directory();
    int file_count = vfs::read_directory(root_dir);
    auto *fp = root_dir->file_list->get_start_node();
    debug::out::printf("files on the root directory ------------\n");
    while(fp != nullptr) {
        file_info *file = fp->object;

        debug::out::printf("%s\n" , file->file_name);
        fp = fp->next;
    }
    
    InputReader kbd_reader;
    kbd_reader.open("keyboard");
    while(1) {
        input_event event;
        if(kbd_reader.read(event)) {
            debug::out::printf("event received : 0x%x(%d)\n" , event.data , event.type);
        }
    }
    
    debug::out::printf("memory usage : %dKB\n" , memory::pmem_usage()/1024);
}

block_device *find_root_block_device(block_device *ramdisk) {
    // No ramdisk found. For now, just panic
    if(ramdisk == nullptr) {
        debug::out::printf("No ramdisk found.\n");
        return nullptr;
    }

    // The ramdisk has a file system, usable for root device
    if(ramdisk->storage_info.fs_driver) return ramdisk;
    
    block_device *logical_devices = nullptr;
    if(ramdisk->storage_info.logical_block_devs) {
        debug::out::printf("Number of partitions : %d\n" , ramdisk->storage_info.logical_block_devs->size());
        for(int i = 0; i < ramdisk->storage_info.logical_block_devs->size(); i++) {
            block_device *bdev_logical = ramdisk->storage_info.logical_block_devs->get(i);
            debug::out::printf("fs_driver : %llx\n" , bdev_logical->storage_info.fs_driver);
            if(bdev_logical->storage_info.fs_driver) {
                debug::out::printf("Logical block device found(%s%d, partition=%d) : %s\n" , 
                    bdev_logical->driver->driver_name , 
                    bdev_logical->id , 
                    bdev_logical->storage_info.partition_id , 
                    bdev_logical->storage_info.fs_driver->fs_string);
                logical_devices = bdev_logical;
                break;
            }
        }
    }
    // No logical devices, and no file system --> ramdisk is unavailable
    return logical_devices;
}