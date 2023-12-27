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
    debug::out::printf("Physical memory allocation test\n");
    max_t alloc_size[100] = {
        2212 , 2565 , 1836 , 1376 , 1564 , 2976 , 2279 ,  745 , 2402 , 1144 ,  
        3836 , 2678 , 3345 , 1841 , 1825 , 3023 , 3334 , 2322 ,  938 , 1689 ,  
        3157 , 1280 ,  562 , 1284 , 3498 ,  813 ,  972 , 2899 , 2205 , 3500 ,  
        2834 , 2654 , 1249 ,  886 , 2655 , 3951 , 3519 , 1923 , 1817 , 586  , 
        3357 , 1262 , 1254 , 3899 , 2415 , 2535 , 2000 ,  701 , 1978 , 1328 ,  
        1820 , 3524 , 2086 , 2938 , 2948 ,  736 , 3059 , 1987 , 1699 , 3461 ,  
        1227 , 3786 ,  887 , 3183 , 3732 , 3506 , 1006 ,  820 , 1982 , 3807 ,  
        4041 ,  693 , 3539 , 2491 , 3371 ,  831 , 1702 , 1562 , 1899 , 3781 ,  
        1977 , 1838 , 2393 , 3018 , 3251 , 1903 , 1320 , 3390 , 3639 , 523  , 
         882 , 2945 , 3362 , 3834 , 1350 , 2954 ,  816 , 2001 ,  839 , 1668
    };
    debug::out::raw_printf("current usage : %d\n" , memory::pmem_usage());
    max_t ptr_list[100] = {0 , };
    dword demanded = 0;
    for(int i = 0; i < 8; i++) {
        max_t ptr = (max_t)memory::pmem_alloc(alloc_size[i]);
        debug::out::raw_printf("%d : 0x%X - 0x%X\n" , i , ptr , ptr+alloc_size[i]);
        ptr_list[i] = ptr;
        demanded += alloc_size[i];
    }
    debug::out::raw_printf("physical usage       : %d\n" , memory::pmem_usage());
    debug::out::raw_printf("allocation requested : %d\n" , demanded);
    for(int i = 0; i < 8; i++) {
        memory::pmem_free((ptr_t *)ptr_list[i]);
        debug::out::raw_printf("%d : Free 0x%X - usage %d\n" , i , ptr_list[i] , memory::pmem_usage());
    }
    // segmentation::init();
    // interrupt::init();
    // paging::init();
    while(1) {
        ;
    }
}