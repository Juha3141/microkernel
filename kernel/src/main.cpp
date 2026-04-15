#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/kasan.hpp>
#include <kernel/mem/pages_manager.hpp>

#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/io_port.hpp>
#include <kernel/driver/block_device_driver.hpp>
#include <kernel/driver/pci.hpp>
#include <ramdisk/ramdisk.hpp>
#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>

#include <kernel/debug.hpp>

#include <kernel/sections.hpp>
#include <loader/loader_argument.hpp>

// For testing

#include <random.hpp>
#include <hash_table.hpp>
#include <pair.hpp>
#include <arch/switch_context.hpp>

memory::Boundary setup_kasan_shadowmem(PageTableData &page_table_data , max_t kernel_size , max_t kernel_stack_size);
memory::Boundary map_usable_mem_into_vaddr(PageTableData &page_table_data , max_t linear_addr_start , max_t required_size , max_t flag=MEMORYMAP_USABLE);
max_t get_kernel_memory_pool_size();

struct MappedRegionInfo {
    max_t used_phys_addr_start;
    max_t used_phys_addr_end;
    max_t mapped_size;
    max_t page_size;
};

__no_sanitize_address__ MappedRegionInfo map_pages_auto_alignment(PageTableData &page_table_data , max_t phys_addr_start , max_t phys_addr_end , max_t linear_addr_start , max_t flags);
__no_sanitize_address__ void update_pt_space_to_kmemmap(void);

extern "C" void sanitized_kernel_main(LoaderArgument *loader_argument);

__no_sanitize_address__
extern "C" void kernel_main(LoaderArgument *loader_argument , max_t kernel_vmem_addr , max_t kernel_stack_vmem_addr , max_t kernel_stack_size) {
    memory::kstruct_init({loader_argument->kstruct_mem_location , loader_argument->kstruct_mem_location+loader_argument->kstruct_mem_size});
    memory::kmemmap_init(loader_argument);

    // Very very temporary!!
    PageTableData page_table_data = {
        .cr3_base = (x86_page_entry_t *)page::get_pt_space_boundary().start_address
    };
    /* 1. Set up the virtual memory for KASan shadow memory space
     * Using the map_usable_mem_into_vaddr() function, utilize the segmented free memory space and map them into 
     * one contiguous virtual memory space that will be used for shadow mem.

     * The memory space will be located at CONFIG_KERNEL_KASAN_VMA. The actual shadow memory starts at the address
     * CONFIG_KERNEL_KASAN_VMA + CONFIG_KERNEL_VMA >> KASAN_SHADOW_SHIFT
     */
#ifdef CONFIG_USE_KASAN
    memory::Boundary kasan_vmem_boundary = setup_kasan_shadowmem(page_table_data , loader_argument->kernel_size , kernel_stack_size);
    max_t kasan_vmem_size = kasan_vmem_boundary.end_address - kasan_vmem_boundary.start_address;
#endif
    /* 2. Set up the free pool virtual memory space
     * Just like setting up KASan shadow memory space, set up the large(almost the entirety of) memory chunks into
     * contiguous virutal memory space. This large pool will exist after where the kernel and kernel stack is located.
     */
    max_t free_pool_vma_start = align_round_up(kernel_stack_vmem_addr+kernel_stack_size , ARCH_MAXIMUM_PAGE_SIZE);
    max_t free_pool_size      = get_kernel_memory_pool_size();
    auto kernel_free_pool_vma = map_usable_mem_into_vaddr(page_table_data , free_pool_vma_start , free_pool_size);
    // TO-DO: Make a similar version of kmemmap, but for the virtual memory mapping!

#ifdef CONFIG_USE_KASAN
    kasan::init(kasan_vmem_size , kernel_free_pool_vma.start_address , kernel_free_pool_vma.end_address);
#endif

    debug::init(loader_argument);
    debug::out::clear_screen(0x00);
    debug::out::printf("Hello world from the higher-half kernel!\n");
    debug::out::printf("Kernel Free Pool Boundary : 0x%llx ~ 0x%llx (%lldMB)\n" , kernel_free_pool_vma.start_address , kernel_free_pool_vma.end_address
         , (kernel_free_pool_vma.end_address-kernel_free_pool_vma.start_address)/1024/1024);
    
    debug::out::printf("========================== Kernel memory map ==========================\n");
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();
    while(kmemmap_ptr != nullptr) {
        debug::out::printf("0x%-16llx ~ 0x%-16llx % 13lldkB (%s)\n" , kmemmap_ptr->start_address , kmemmap_ptr->end_address , (kmemmap_ptr->end_address-kmemmap_ptr->start_address)/1024 , memory::memmap_type_to_str(kmemmap_ptr->type));
        kmemmap_ptr = kmemmap_ptr->next;
    }
    debug::out::printf("Actually usable kernel memory : %lldMB\n" , (kernel_free_pool_vma.end_address - kernel_free_pool_vma.start_address)/1024/1024);

    memory::pmem_init(free_pool_vma_start , free_pool_vma_start+free_pool_size);
    sanitized_kernel_main(loader_argument);
    while(1) {
        ;
    }
}

extern "C" void sanitized_kernel_main(LoaderArgument *loader_argument) {
    debug::out::printf(DEBUG_INFO , "----- Initializing segmentation system..\n");
    segmentation::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing interrupt system..\n");
    interrupt::init();
    exception::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing block device driver..\n");
    blockdev::init();
    storage_system::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing file system driver..\n");
    fsdev::init();
    debug::out::printf(DEBUG_INFO , "----- Initializing character device driver..\n");
    chardev::init();
    register_file_system_drivers();
    register_kernel_drivers();

    debug::out::printf(DEBUG_INFO , "----- Initializing vfs..\n");
    debug::out::printf(DEBUG_INFO , "Setting root directory to the provided ramdisk : 0x%lx-0x%lx\n" , loader_argument->ramdisk_location , loader_argument->ramdisk_location+loader_argument->ramdisk_size);
    // find the ramdisk driver
    if(loader_argument->is_ramdisk_available) {
        blockdev::block_device *device = ramdisk_driver::create(loader_argument->ramdisk_size/512 , 512 , loader_argument->ramdisk_location);
        if(blockdev::register_device(device->device_driver->driver_id , device) != INVALID) {
            // mount to the root device
            vfs::init(device);
        }
    }
    else {
        debug::out::printf("no ramdisk found!\n");
    }
    debug::out::printf("memory usage : %dKB\n" , memory::pmem_usage()/1024);

    file_info *root_dir = vfs::get_root_directory();
    int file_count = vfs::read_directory(root_dir);
    auto *fp = root_dir->file_list->get_start_node();
    while(fp != nullptr) {
        file_info *file = fp->object;

        debug::out::printf("%s\n" , file->file_name);
        fp = fp->next;
    }
    
    pci::probe_all_pci_devices();
    
    debug::out::printf("We're currently in safe mode\n");
}

#define PAGE_COUNT_THRESHOLD (CONFIG_LARGE_PAGE_SIZE/CONFIG_PAGE_SIZE)*4

/// @brief get_kernel_memory_pool_size() scans the size of the available memory that kernel will use as a free pool from the
///        kernel's global memory map(global_kmemmap)
///        The size of the memory pool is in multiple of CONFIG_PAGE_SIZE. That is, each chunks of the memory is rounded down to the
///        nearest multiple of the CONFIG_PAGE_SIZE. (align_round_down)
/// @return Size of the available kernel memory (NOT number of pages)
__no_sanitize_address__ 
max_t get_kernel_memory_pool_size() {
    max_t kernel_memory_pool_size = 0;
    KernelMemoryMap *ptr = memory::global_kmemmap();
    while(ptr != nullptr) {
        max_t len = (ptr->end_address-ptr->start_address);
        // skip if the size of the memory region is smaller than page size
        if(ptr->type != MEMORYMAP_USABLE) { ptr = ptr->next; continue; }

        kernel_memory_pool_size += align_round_down(len , CONFIG_PAGE_SIZE);
        ptr = ptr->next;
    }
    return kernel_memory_pool_size;
}

/// @brief Automatically map the memory chunks that's identified as usable in the kernel global memory map 
///        to the given linear address for given amount of size.
///        The result is the continuous linear address space with given size(required_size). If the flag is given, the flag of the 
///        used memory chunks will be changed into the given flag. 
///        In summary, this function will scan the kernel memory map, use any usable memory chunk to construct a continuous linear space
///        from address linear_addr_start  to linear_addr_start+required_size
///        This function uses page::map_pages to map the page, alongside with the alloc_pt_space function for page table space allocation
/// @param page_table_data    Page table data required for page::map_pages
/// @param linear_addr_start  Start address of the linear address space
/// @param required_size      Size for the linear address space that'll be newly mapped
/// @param flag               If the chunks needs to have be marked occupied after being used
/// @return Linear address space boundary that is being mapped
__no_sanitize_address__
memory::Boundary map_usable_mem_into_vaddr(PageTableData &page_table_data , max_t linear_addr_start , max_t required_size , max_t flag) {
    max_t linear_address_mapping_location = linear_addr_start;
    // variable tracking how many number of pages the system has mapped
    max_t mapped_memory_size = 0;
    KernelMemoryMap *kmemmap_ptr = memory::global_kmemmap();
    int i = 0;
    KernelMemoryMap kasan_kmemmap[256];
    update_pt_space_to_kmemmap();
    while(kmemmap_ptr != nullptr) {
        max_t addr_start = kmemmap_ptr->start_address;
        max_t addr_end   = kmemmap_ptr->end_address;
        if(kmemmap_ptr->type != MEMORYMAP_USABLE) { kmemmap_ptr = kmemmap_ptr->next; continue; }
        max_t chunk_map_size = min(mapped_memory_size+addr_end-addr_start , required_size)-mapped_memory_size;
        
        // debug::out::printf("map_pages_auto_alignment() : paddr=0x%-15llx 0x%-15llx,  laddr=0x%-15llx\n" , addr_start , addr_start+chunk_map_size , linear_address_mapping_location);
        MappedRegionInfo region_info = map_pages_auto_alignment(
            page_table_data , 
            addr_start , 
            addr_start+chunk_map_size , 
            linear_address_mapping_location , 
            PAGE_ENTRY_FLAGS_PRESENT|PAGE_ENTRY_FLAGS_KERNEL|PAGE_ENTRY_FLAGS_RW
        );
        max_t page_size = region_info.page_size;
        
        kasan_kmemmap[i++] = {
            .start_address = region_info.used_phys_addr_start , 
            .end_address   = region_info.used_phys_addr_end , 
            .type = static_cast<unsigned int>(flag)
        };

        linear_address_mapping_location += chunk_map_size;
        mapped_memory_size              += chunk_map_size;

        // If the mapped page count is bigger than the number of pages of the calculated size of shadowmem, 
        // we have completed mapping all the shadowmem.
        if(mapped_memory_size >= required_size) {
            break;
        }
        kmemmap_ptr = kmemmap_ptr->next;
    }
    for(int k = 0; k < i; k++) {
        if(!memory::add_kmemmap_entry(kasan_kmemmap[k])) {
            debug::panic_line(__FILE_NAME__ , __LINE__ , "add_kmemmap_entry() failed, arg1: %llx, arg2: %llx, arg3: %d\n" , 
                kasan_kmemmap[k].start_address , kasan_kmemmap[k].end_address , flag);
        }
    }
    return {linear_addr_start , linear_address_mapping_location};
}

/// @brief Update the PT(page table) space to the global kernel memory map, using get_pt_space_boundary()
///        If there already exists PT space entry in the memory map, update the entry and adjacent ones. 
///        If there exists none, add the entry to the map.
__no_sanitize_address__ 
void update_pt_space_to_kmemmap(void) {
    static KernelMemoryMap *pt_space_entry = nullptr;
    KernelMemoryMap *ptr = memory::global_kmemmap();
    auto pt_space_boundary = page::get_pt_space_boundary();
    if(pt_space_entry == nullptr) {
        bool found = false;
        while(ptr != nullptr) {
            if(ptr->type == MEMORYMAP_KERNEL_PT_SPACE) {
                pt_space_entry = ptr;
                found = true;
                break;
            }
            ptr = ptr->next;
        }
        if(!found) {
            pt_space_entry = memory::add_kmemmap_entry(
                (KernelMemoryMap){
                    .start_address = pt_space_boundary.start_address , 
                    .end_address   = pt_space_boundary.end_address , 
                    .type          = MEMORYMAP_KERNEL_PT_SPACE
                });
        }
        return;
    }
    
    pt_space_entry->end_address         = pt_space_boundary.end_address;
    pt_space_entry->next->start_address = pt_space_boundary.end_address;
}

__no_sanitize_address__
memory::Boundary setup_kasan_shadowmem(PageTableData &page_table_data , max_t kernel_size , max_t kernel_stack_size) {
    // Get a space for KASan first
    max_t kernel_memory_pool_size = get_kernel_memory_pool_size();
    max_t page_size = 
#if CONFIG_USE_LARGE_PAGE == yes
    CONFIG_LARGE_PAGE_SIZE;
#else
    CONFIG_PAGE_SIZE;
#endif

 // max_t kernel_area_to_be_protected_start = 0;
    max_t kernel_area_to_be_protected_end   = 0;
    KernelMemoryMap *ptr = memory::global_kmemmap();
    while(ptr != nullptr) {
        switch(ptr->type) {
            case MEMORYMAP_KERNEL_IMAGE:
            case MEMORYMAP_KERNEL_STACK:
            case MEMORYMAP_LOADER_ARGUMENT:
            case MEMORYMAP_KSTRUCT_POOL:
                kernel_area_to_be_protected_end = max(kernel_area_to_be_protected_end , ptr->end_address);
                break;
        }
        ptr = ptr->next;
    }
    // debug::out::printf("Kernel area to be protected end : 0x%llx\n" , kernel_area_to_be_protected_end);
    max_t kernel_area_to_be_protected_vma  = CONFIG_KERNEL_KASAN_VMA;
    max_t kernel_area_to_be_protected_size = align_round_up((kernel_area_to_be_protected_end+KASAN_GRANUL_SIZE) >> KASAN_SHADOW_SHIFT , CONFIG_PAGE_SIZE);
    map_usable_mem_into_vaddr(page_table_data  , kernel_area_to_be_protected_vma , kernel_area_to_be_protected_size , MEMORYMAP_KASAN_SHADOWMEM);
    // debug::out::printf("KASan shadow mem front : 0x%llx ~ 0x%llx\n" , kernel_area_to_be_protected_vma , kernel_area_to_be_protected_vma+kernel_area_to_be_protected_size);
    
    max_t kasan_shadowmem_kernel_pool_vma = CONFIG_KERNEL_KASAN_VMA + (CONFIG_KERNEL_VMADDRESS >> KASAN_SHADOW_SHIFT);
    max_t kasan_shadowmem_kernel_pool_size = align_round_up(kernel_memory_pool_size/(KASAN_GRANUL_SIZE+1) , DEFAULT_PAGE_SIZE)
                                 + align_round_up(kernel_size       , page_size)
                                 + align_round_up(kernel_stack_size , page_size);
    // debug::out::printf("KASan shadow mem kernel pool : 0x%llx ~ 0x%llx\n" , kasan_shadowmem_kernel_pool_vma , kasan_shadowmem_kernel_pool_vma+kasan_shadowmem_kernel_pool_size);
    // debug::out::printf("Kernel memory pool size  : %dMB\n" , kernel_memory_pool_size/1024/1024);
    // debug::out::printf("KASan shadow memory size : %dMB\n" , kasan_shadowmem_size/1024/1024);
    // debug::out::printf("KASan memory area : 0x%-15llx 0x%-15llx\n" , kasan_shadowmem_addr_kernel_vma , kasan_shadowmem_addr_kernel_vma+kasan_shadowmem_size);
    auto res = map_usable_mem_into_vaddr(page_table_data , kasan_shadowmem_kernel_pool_vma , kasan_shadowmem_kernel_pool_size , MEMORYMAP_KASAN_SHADOWMEM);

    max_t mapped_memory_size = (res.end_address - res.start_address);
    // debug::out::printf("Total mapped size           : %dkB (%d.%d%d%%)\n" , mapped_memory_size/1024 , 
        // ((mapped_memory_size*100)/kernel_memory_pool_size) , ((mapped_memory_size*1000)/kernel_memory_pool_size)%10 , ((mapped_memory_size*10000)/kernel_memory_pool_size)%10);
    
    return res;
}

#define LARGE_PS_THRESHOLD ((CONFIG_LARGE_PAGE_SIZE)*4ULL)
#define ENORMOUS_PS_THRESHOLD ((CONFIG_ENORMOUS_PAGE_SIZE)*4ULL)

/// @brief 
/// @param page_table_data 
/// @param phys_addr_start 
/// @param phys_addr_end 
/// @param linear_addr_start 
/// @param flags 
/// @return 
__no_sanitize_address__ MappedRegionInfo map_pages_auto_alignment(PageTableData &page_table_data , max_t phys_addr_start , max_t phys_addr_end , max_t linear_addr_start , max_t flags) {
    if(phys_addr_start == phys_addr_end) return {};
    // default will be levels_of_page_size[0]
    const max_t levels_of_page_size[] = {
        CONFIG_PAGE_SIZE , 
        CONFIG_LARGE_PAGE_SIZE , 
        CONFIG_ENORMOUS_PAGE_SIZE , 
    };
    const max_t levels_of_ps_threshold[] = {
        0 , 
        LARGE_PS_THRESHOLD , 
        ENORMOUS_PS_THRESHOLD , 
    };
    constexpr int lps_sz  = sizeof(levels_of_page_size)/sizeof(levels_of_page_size[0]);
    constexpr int lpst_sz = sizeof(levels_of_ps_threshold)/sizeof(levels_of_ps_threshold[0]);

    max_t required_map_size = phys_addr_end - phys_addr_start;
    int page_size_idx = -1;
    /** 1. Determine page size */
    for(int i = lpst_sz-1; i >= 0; i--) {
        // If it exceeds 
        if(required_map_size > levels_of_ps_threshold[i]) {
            page_size_idx = i;
            break;
        }
    }
    // debug::out::printf("page size : 0x%llx(%dth from the hierarchy)\n" , levels_of_page_size[page_size_idx] , page_size_idx);
    
    max_t phys_addr_offset   = 0;
    max_t linear_addr_offset = 0;
    max_t page_size = levels_of_page_size[page_size_idx];

    max_t addr_size = phys_addr_end-phys_addr_start;
    
    max_t aligned_phys_addr   = align_round_up(phys_addr_start , page_size);
    max_t alignment_padding_phys = aligned_phys_addr-phys_addr_start;
    max_t aligned_linear_addr = align_round_up(linear_addr_start , page_size);
    max_t alignment_padding_linear = aligned_linear_addr-linear_addr_start;

    max_t aligned_phys_end = align_round_down(phys_addr_end , page_size);
    max_t alignment_padding_phys_end = phys_addr_end-aligned_phys_end;

    max_t aligned_linear_end = align_round_down(linear_addr_start+addr_size , page_size);
    max_t alignment_padding_linear_end = linear_addr_start+addr_size-aligned_linear_end;

    if(alignment_padding_phys < alignment_padding_linear) {
        alignment_padding_phys += page_size;
        aligned_phys_addr      += page_size;
        
        // The region is too small to align
        if(aligned_phys_addr > aligned_phys_end) return {};
    }
    map_pages_auto_alignment(page_table_data , phys_addr_start , phys_addr_start+alignment_padding_linear , linear_addr_start , flags);

    // debug::out::printf("aligned_phys_addr   : 0x%-13llx ~ 0x%-13llx (sz=0x%llx)\n" , aligned_phys_addr , aligned_phys_end , aligned_phys_end-aligned_phys_addr);
    // debug::out::printf("padding             : 0x%llx\n" , alignment_padding_phys);
    // debug::out::printf("aligned_linear_addr : 0x%-13llx ~ 0x%-13llx (sz=0x%llx)\n" , aligned_linear_addr , aligned_linear_end , aligned_linear_end-aligned_linear_addr);
    // debug::out::printf("padding             : 0x%llx\n" , alignment_padding_linear);

    max_t page_count = (aligned_linear_end-aligned_linear_addr)/page_size;
    page::map_pages(page_table_data , aligned_linear_addr , page_size , page_count , aligned_phys_addr , flags , page::alloc_pt_space);
    // debug::out::printf("pages::map_pages() : laddr=%-15llx~%-14llx paddr=%-14llx~%-14llx ps=%lld sz=%lld pages\n" , 
        // aligned_linear_addr , aligned_linear_end , aligned_phys_addr , aligned_phys_addr+(page_count*page_size) , page_size , page_count);

    map_pages_auto_alignment(page_table_data , aligned_phys_end , phys_addr_end , aligned_linear_end , flags);

    return {
        .used_phys_addr_start = phys_addr_start , 
        .used_phys_addr_end   = phys_addr_end , 
        .mapped_size            = phys_addr_end-phys_addr_start , 
        .page_size              = page_size
    };
}