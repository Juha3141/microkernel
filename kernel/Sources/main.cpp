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
    debug::init();
    debug::out::clear_screen(0x07);
    debug::push_function("kmain");

    struct KernelArgument *kargument = (struct KernelArgument *)kernel_info_struct_addr;
    debug::out::printf("Dumping memory map : \n");
    struct MemoryMap *memmap = (struct MemoryMap *)(kargument->memmap_ptr);
    for(int i = 0; i < kargument->memmap_count; i++) {
        max_t addr = ((max_t)memmap[i].addr_high << 32)|memmap[i].addr_low;
        max_t len = ((max_t)memmap[i].length_high << 32)|memmap[i].length_low;
        debug::out::printf("0x%lX~0x%lX (%d)\n" , addr , addr+len , memmap[i].type);
    }
    if(kargument->signature != KERNELINFO_STRUCTURE_SIGNATURE) {
        debug::out::printf("Kernel structure not found!\n");
        debug::panic("kernel structure not found");
    }
    debug::out::printf(DEBUG_INFO , "kernel code location : 0x%X~0x%X\n" , kargument->kernel_address , kargument->kernel_address+kargument->kernel_size);
    memory::pmem_init(kargument->memmap_count , (struct MemoryMap *)((max_t)kargument->memmap_ptr) , kargument);
    set_initial_kernel_info();

    interrupt::hardware::disable();
    segmentation::init();
    interrupt::init();
    exception::init();
    
    segmentation::SegmentsManager *segmentation_mgr = segmentation::SegmentsManager::get_self();
    for(int i = 0; i < segmentation_mgr->segments_count; i++) {
        debug::out::printf("seg%d. 0x%02x - %s\n" , i , segmentation_mgr->segments_data[i].value , segmentation_mgr->segments_data[i].name);
    }

    debug::out::printf("hello world\n");
    
    //interrupt::hardware::enable();

    int i = 10;
    int b = i-10;
    int c = i/b;

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