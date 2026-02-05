#include <page_table.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <string.hpp>
#include <kernel/debug.hpp>

#include <x86_64/cpuid.hpp>

qword set_pml4_entry(qword physical_address , dword flags) { return physical_address|flags; }

constexpr max_t pml5t_single_page_size = (max_t)512*512*1024*1024*1024;
constexpr max_t pml4t_single_page_size = (max_t)512*1024*1024*1024;
constexpr max_t pdpt_single_page_size  = (max_t)1024*1024*1024;
constexpr max_t pde_single_page_size   = (max_t)2*1024*1024;
constexpr max_t pte_single_page_size    = (max_t)4*1024;

constexpr max_t entry_sz = 8;

#define CEIL(numerator , denominator) (((max_t)(numerator)/denominator)+((numerator)%(denominator) != 0))

constexpr max_t pdpt_total_size(max_t phys_addr_end) { return CEIL(phys_addr_end , pdpt_single_page_size)*entry_sz; }
constexpr max_t pde_total_size(max_t phys_addr_end)  { return CEIL(phys_addr_end , pde_single_page_size)*entry_sz; }
constexpr max_t pe_total_size(max_t phys_addr_end) { return CEIL(phys_addr_end , pte_single_page_size)*entry_sz; }

max_t ARCHDEP page::calculate_identity_page_table_size(max_t phys_addr_end) {
    debug::out::printf("phys_addr_end : 0x%016x\n" , phys_addr_end);

    max_t occupied_space = pdpt_total_size(phys_addr_end)
                         + pde_total_size(phys_addr_end)
                         + pe_total_size(phys_addr_end);
    debug::out::printf("                            Total %dkB\n" , occupied_space/1024);

    return occupied_space;
}

struct x86_page_tables_num {
    max_t pml4t_num;
    max_t pdpt_num;
    max_t pde_num;
    max_t pt_num;
};

static x86_page_tables_num translate_linear_addr(max_t linear_addr) {
    return {
        .pml4t_num = (linear_addr >> 39) & 0x1ff , 
        .pdpt_num  = (linear_addr >> 30) & 0x1ff , 
        .pde_num   = (linear_addr >> 21) & 0x1ff , 
        .pt_num    = (linear_addr >> 12) & 0x1ff
    };
}

/*
PAGE_ENTRY_FLAGS_PRESENT   0x01
PAGE_ENTRY_FLAGS_KERNEL    0x02
PAGE_ENTRY_FLAGS_USER      0x04
PAGE_ENTRY_FLAGS_READ_ONLY 0x08
PAGE_ENTRY_FLAGS_RW        0x10
PAGE_ENTRY_FLAGS_EXD       0x20
*/

static max_t generate_page_entry_flag(max_t flag) {
    max_t page_entry_flag = 0;
    if((flag & PAGE_ENTRY_FLAGS_PRESENT) == PAGE_ENTRY_FLAGS_PRESENT) page_entry_flag |= PML4_FLAGS_PRESENT;
    if((flag & PAGE_ENTRY_FLAGS_USER) == PAGE_ENTRY_FLAGS_USER)       page_entry_flag |= PML4_FLAGS_US;
    if((flag & PAGE_ENTRY_FLAGS_RW) == PAGE_ENTRY_FLAGS_RW)           page_entry_flag |= PML4_FLAGS_RW;
    if((flag & PAGE_ENTRY_FLAGS_EXD) == PAGE_ENTRY_FLAGS_EXD)         page_entry_flag |= PML4_FLAGS_EXD;
    return page_entry_flag;
}

// By the way, the implementation of map_one_page() right now feels kinda dumb. I hope you fix it sooner or later, Ian. 

/// @brief Set one page entry corresponding to the linear address/page size by provided physical address and flags
/// @param page_table_data Architecture-dependent page table metadata 
/// @param linear_address 
/// @param page_size Size of one page
/// @param physical_address Physical address that the corresponding page entry will be mapped
/// @param flags Flags
/// @param alloc_func The allocator that will be used to allocate memory space for page tables
__no_sanitize_address__ void ARCHDEP page::map_one_page(PageTableData &page_table_data , max_t linear_address , max_t page_size , 
        max_t physical_address , max_t flags , func_alloc_pt_space_t alloc_func) {
    // translate linear page number into pde, pdpt, and pde numbers

    // In Theory, PML4 paging can support up to 256EBs of physical memory
    // One PML4 Entry can handle 512GBs of physical memory (1GB*512 = 512GB)
    // One PDPT Entry can handle 1GBs of physical memory (2MB*512 = 1GB)
    // One PDE Entry can handle 2MBs of physical memory (4KB*512  = 2MB)
    
    // debug::out::printf("la : 0x%llx, pa : 0x%llx, ps : 0x%llx, f=%d\n" , linear_address , physical_address , page_size , flags);
    if(page_size != pte_single_page_size
    && page_size != pde_single_page_size
    && page_size != pdpt_single_page_size
    && page_size != pml4t_single_page_size
    && page_size != pml5t_single_page_size) {
        // kernel panic
        debug::panic("page::map_one_page(): Invalid page size requested, page_size = %lld\n" , page_size);
    }

    auto [pml4t_num, pdpt_num, pde_num, pte_num] = translate_linear_addr(linear_address);
    // debug::out::printf("[%d, %d, %d, %d]\n" , pml4t_num , pdpt_num , pde_num , pte_num);
    
    // Set up for PML4 entry
    if(page_table_data.cr3_base == nullptr) {
        page_table_data.cr3_base = (x86_page_entry_t *)alloc_func(512*8 , 4096);
        memset(page_table_data.cr3_base , 0 , sizeof(x86_page_entry_t)*512*8);
        // debug::out::printf("setting up pml4 entry(cr3 base) : 0x%llx\n" , page_table_data.cr3_base);
    }
    
    // only get the address to pdpt from the entry, excluding the EXB flag(at the bit 63)
    uint64_t addr_mask = (uint64_t)((~(((uint64_t)1 << 12)-1))^((uint64_t)1 << 63));
    x86_page_entry_t *pdpt_entries = (x86_page_entry_t *)(page_table_data.cr3_base[pml4t_num] & addr_mask);

    // Set up for PDPT entry
    if(pdpt_entries == nullptr) {
        pdpt_entries = (x86_page_entry_t *)alloc_func(512*8 , 4096);
        // debug::out::printf("new_pdpt_table : 0x%llx\n" , (uint64_t)pdpt_entries);
        page_table_data.cr3_base[pml4t_num] = (uint64_t)pdpt_entries|generate_page_entry_flag(flags);
    }
    // If the desired page size is one pdpt entry, set PS=1 and 
    if(page_size == pdpt_single_page_size) {
        pdpt_entries[pdpt_num] = physical_address|generate_page_entry_flag(flags)|PML4_FLAGS_PS;
        return;
    }

    // final level (PML4)
    x86_page_entry_t *pde_entries = (x86_page_entry_t *)(pdpt_entries[pdpt_num] & addr_mask);

    // pdpt_entries[pdpt_num] = target PDE entry
    if(pde_entries == nullptr) {
        pde_entries = (x86_page_entry_t *)alloc_func(512*8 , 4096);
        // debug::out::printf("new_pde_table : 0x%llx\n" , (uint64_t)pde_entries);
        pdpt_entries[pdpt_num] = (uint64_t)pde_entries|generate_page_entry_flag(flags);
    }
    if(page_size == pde_single_page_size) {
        pde_entries[pde_num] = physical_address|generate_page_entry_flag(flags)|PML4_FLAGS_PS;
        return;
    }

    if(pde_entries[pde_num] == 0x00) {
        x86_page_entry_t *new_pte_table = (x86_page_entry_t *)alloc_func(512*8 , 4096);
        // debug::out::printf("new_pte_table : 0x%llx\n" , (uint64_t)new_pte_table);
        pde_entries[pde_num] = (uint64_t)new_pte_table|generate_page_entry_flag(flags);
    }

    x86_page_entry_t *pte_entries = (x86_page_entry_t *)(pde_entries[pde_num] & addr_mask);

    pte_entries[pte_num] = physical_address|generate_page_entry_flag(flags);
}

__no_sanitize_address__ void ARCHDEP page::register_page_table(PageTableData &page_table_data) {
    dword eax , unused;
    cpuid(0x80000008 , eax , unused , unused , unused);
    debug::out::printf("CPUID.0x80000008 = 0x%08x, " , eax);
    word max_linear_addr_width = (eax >> 8) & 0xFF;
    
    debug::out::printf("max_linear_addr_width = %d\n" , max_linear_addr_width);
    
    // 5-level paging is supported
#if CONFIG_X86_64_LA == 57
    if(max_linear_addr_width == 57) {
        // do whatever
    }
#endif
    debug::out::printf("Previous page table : 0x%llx\n" , fetch_cr3_reg());
    set_cr3_reg((qword)page_table_data.cr3_base);
}

__attribute__ ((naked)) qword  fetch_cr3_reg() {
    __asm__ ("mov rax , cr3");
    __asm__ ("ret");
}

__attribute__ ((naked)) void  set_cr3_reg(qword cr3) {
    __asm__ ("mov cr3 , rdi");
    __asm__ ("ret");
}

__attribute__ ((naked)) void  set_cr4_reg(qword cr4) {
    __asm__ ("mov cr4 , rdi");
    __asm__ ("ret");
}
__attribute__ ((naked)) qword fetch_cr4_reg() {
    __asm__ ("mov rax , cr4");
    __asm__ ("ret");
}