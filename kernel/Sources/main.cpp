#include <kmem_manager.hpp>
#include <debug.hpp>
#include <random.hpp>
#include <interrupt.hpp>
#include <exception.hpp>
#include <segmentation.hpp>
#include <kernel_info.hpp>

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
    
    //interrupt::hardware::enable();

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