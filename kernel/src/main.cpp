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

extern "C" __entry_function__ void kernel_setup(struct LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        if(loader_argument->video_mode == LOADER_ARGUMENT_VIDEOMODE_GRAPHIC) *((max_t *)(max_t)loader_argument->dbg_graphic_framebuffer_addr) = 0xffffffffffffffff;
        else *((byte *)(max_t)loader_argument->dbg_text_framebuffer_addr) = 'E';

        while(1) { ; }
    }
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    debug::init(loader_argument);
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the kernel!\n");

    page::init_pt_space_allocator(loader_argument);
    PageTableData page_table_data;
    page::set_page_entry_vaddr(page_table_data , 0x7fffffff00000000 , 4096 , 0x100000 , PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL);
    // page::set_page_entry(page_table_data , 100 , 4194304 , 2*1024*1024 , 0x00 , PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL);

    while(1) {
        ;
    }
}

extern "C" void kernel_main(struct LoaderArgument *loader_argument) {
    if(loader_argument->signature != LOADER_ARGUMENT_SIGNATURE) {
        if(loader_argument->video_mode == LOADER_ARGUMENT_VIDEOMODE_GRAPHIC) *((max_t *)(max_t)loader_argument->dbg_graphic_framebuffer_addr) = 0xffffffffffffffff;
        else *((byte *)(max_t)loader_argument->dbg_text_framebuffer_addr) = 'E';

        while(1) { ; }
    }
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    debug::init(loader_argument);
    
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the kernel!\n");
    
    MemoryMap *memmap = (MemoryMap *)((max_t)loader_argument->memmap_location);
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