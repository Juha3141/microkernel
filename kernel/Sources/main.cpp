#include <kmem_manager.hpp>
#include <debug.hpp>

#include <kernel_argument.hpp>

extern "C" void kernel_main(unsigned long kernel_info_struct_addr) {
    debug::init();
    debug::out::clear_screen(0x07);
    debug::push_function("kmain");

    struct KernelInfoStructure *kinfostruct = (struct KernelInfoStructure *)kernel_info_struct_addr;
    if(kinfostruct->signature != KERNELINFO_STRUCTURE_SIGNATURE) {
        debug::out::printf("Kernel structure not found!\n");
        debug::panic("kernel structure not found");
    }
    
    debug::out::printf(DEBUG_INFO , "kernel code location : 0x%X~0x%X\n" , kinfostruct->kernel_address , kinfostruct->kernel_address+kinfostruct->kernel_size);
    memory::pmem_init(kinfostruct->memmap_count , (struct MemoryMap *)kinfostruct->memmap_ptr , kinfostruct);
    debug::out::printf(DEBUG_INFO , "hello from kernel main\n");
    
    // segmentation::init();
    // interrupt::init();
    // paging::init();
    while(1) {
        ;
    }
}