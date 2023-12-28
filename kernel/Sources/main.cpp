#include <kmem_manager.hpp>
#include <debug.hpp>
#include <random.hpp>

#include <kernel_argument.hpp>

void pmem_alloc_test(int rand_seed);

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
    // paging::init();
    // segmentation::init();
    // interrupt::init();
    
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
        memset((ptr_t *)ptr , 0xDA , alloc_size[i]);
        debug::out::printf("%d : 0x%X - 0x%X\n" , i , ptr , ptr+alloc_size[i]);
        ptr_list[i] = ptr;
        demanded += alloc_size[i];
    }
    debug::out::printf("physical usage       : %d\n" , memory::pmem_usage());
    debug::out::printf("allocation requested : %d\n" , demanded);
    for(int i = 0; i < 8; i++) {
        memset((ptr_t *)ptr_list[i] , 0 , alloc_size[i]);
        memory::pmem_free((ptr_t *)ptr_list[i]);
        debug::out::printf("%d : Free 0x%X - usage %d\n" , i , ptr_list[i] , memory::pmem_usage());
    }
    debug::pop_function();
}