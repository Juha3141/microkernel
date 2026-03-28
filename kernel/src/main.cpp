#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/io_port.hpp>

#include <kernel/sections.hpp>

#include <loader/loader_argument.hpp>
#include <loader/kernel_setup_argument.hpp>
#include <kernel/mem/pages_manager.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

// + Add kernel_setup argument that tells the virtual addresses of important memory areas like stack, loader_argument and kstruct etc.
extern "C" void kernel_main(LoaderArgument *loader_argument) {
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    debug::init(loader_argument);
    
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the higher-half kernel!\n");
    
    debug::out::printf("Kernel Setup Page Table Space : 0x%-10llx ~ 0x%-10llx\n" , loader_argument->pt_space_start , loader_argument->pt_space_end);

    memory::kmemmap_init(loader_argument);
    memory::add_kmemmap_entry((KernelMemoryMap){
        .start_address = loader_argument->pt_space_start , 
        .end_address   = loader_argument->pt_space_end , 
        .type          = MEMORYMAP_KERNEL_PT_SPACE
    });
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();

    debug::out::printf("========================== Kernel memory map ==========================\n");
    while(kmemmap_ptr != nullptr) {
        debug::out::printf("0x%-16llx ~ 0x%-16llx % 13lldkB (%s)\n" , kmemmap_ptr->start_address , kmemmap_ptr->end_address , (kmemmap_ptr->end_address-kmemmap_ptr->start_address)/1024 , memory::memmap_type_to_str(kmemmap_ptr->type));
        kmemmap_ptr = kmemmap_ptr->next;
    }

    while(1) {
        ;
    }
    /* To-do : Add kernel memmap initialization here, 
     * Also add KASan initialization here before the pmem_init
     */

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