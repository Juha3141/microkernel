#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/sections.hpp>

#include <loader/loader_argument.hpp>
#include <kernel/mem/pages_manager.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

extern "C" void kernel_main(struct LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        while(1) { ; }
    }
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    debug::init(loader_argument);
    
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the kernel!\n");
    while(1) {
        ;
    }

    LoaderMemoryMap *memmap = (LoaderMemoryMap *)((max_t)loader_argument->memmap_location);
    memory::pmem_init(memmap , loader_argument->memmap_count , loader_argument);

    debug::out::printf(DEBUG_INFO , "----- Initializing segmentation system..\n");
    segmentation::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing interrupt system..\n");
    interrupt::init();
    exception::init();

    debug::out::printf("We're currently in safe mode\n");

    while(1) {
        ;
    }
}