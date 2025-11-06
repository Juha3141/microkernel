#include <page_table.hpp>
#include <kernel/mem/pages_manager.hpp>

#include <x86_64/cpuid.hpp>

qword set_pml4_entry(qword physical_address , dword flags) { return physical_address|flags; }

constexpr max_t pml4t_memory_size = (max_t)512*1024*1024*1024;
constexpr max_t pdpt_memory_size  = (max_t)1024*1024*1024;
constexpr max_t pde_memory_size   = (max_t)2*1024*1024;
constexpr max_t pe_memory_size    = (max_t)4*1024;

constexpr max_t entry_sz = 8;

#define CEIL(numerator , denominator) (((max_t)(numerator)/denominator)+((numerator)%(denominator) != 0))

constexpr max_t pdpt_total_size(max_t phys_addr_end) { return CEIL(phys_addr_end , pdpt_memory_size)*entry_sz; }
constexpr max_t pde_total_size(max_t phys_addr_end)  { return CEIL(phys_addr_end , pde_memory_size)*entry_sz; }
constexpr max_t pe_total_size(max_t phys_addr_end) { return CEIL(phys_addr_end , pe_memory_size)*entry_sz; }

max_t ARCHDEP page::calculate_identity_page_table_size(max_t phys_addr_end) {
    debug::out::printf("phys_addr_end : 0x%016x\n" , phys_addr_end);

    max_t occupied_space = pdpt_total_size(phys_addr_end)
                         + pde_total_size(phys_addr_end)
                         + pe_total_size(phys_addr_end);
    debug::out::printf("                            Total %dkB\n" , occupied_space/1024);

    return occupied_space;
}

max_t ARCHDEP page::init_identity_page_table(PageTableData &page_table_data , max_t page_table_location , max_t phys_addr_end) {
    page_table_data.base_location = page_table_location;

    max_t pml4_table_count  = CEIL(phys_addr_end , pml4t_memory_size);
    max_t pdp_table_count   = CEIL(phys_addr_end , pdpt_memory_size);
    max_t pde_table_count   = CEIL(phys_addr_end , pde_memory_size);
    max_t pe_table_4k_count = CEIL(phys_addr_end , pe_memory_size);
    
    qword *pml4_table = (qword *)alloc_pt_space(PML4_ENTRY_MAXCOUNT*8 , 4096);
    
    for(int i = 0; i < pml4_table_count; i++) {
        qword *pdpt_table = (qword *)alloc_pt_space(PML4_ENTRY_MAXCOUNT*8 , 4096);
        pml4_table[i] = set_pml4_entry((max_t)pdpt_table , 0);
    }
    
    return 0;
}

const max_t ARCHDEP page_size() {
#ifdef CONFIG_X86_64_USE_5_LEVEL_PAGING
    return 4096;
#else
    return 0x200000;
#endif
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

void ARCHDEP page::set_page_entry_vaddr(
        PageTableData &page_table_data , 
        max_t linear_addr , 
        max_t page_size , 
        max_t phyiscal_addr , 
        max_t flags) {
    auto [pml4t_num, pdpt_num, pde_num, pt_num] = translate_linear_addr(linear_addr);
    
    if(page_table_data.pml4_entry == nullptr) {
        page_table_data.pml4_entry = (pml4_entry_t *)alloc_pt_space(512*8 , 4096);
    }
    
    if(page_table_data.pml4_entry[pml4t_num] == 0x00) {
        
    }
}

void ARCHDEP page::set_page_entry(PageTableData &page_table_data , max_t linear_page_number , max_t pages_count , max_t page_size , 
        max_t physical_address , max_t flags) {
    // translate linear page number into pde, pdpt, and pde numbers

    // In Theory, PML4 paging can support up to 256EBs of physical memory
    // One PML4 Entry can handle 512GBs of physical memory (1GB*512 = 512GB)
    // One PDPT Entry can handle 1GBs of physical memory (2MB*512 = 1GB)
    // One PDE Entry can handle 2MBs of physical memory (4KB*512  = 2MB)
}

void ARCHDEP page::register_page_table(PageTableData &page_table_data) {
    dword eax , unused;
    cpuid(0x80000008 , eax , unused , unused , unused);
    debug::out::printf("CPUID.0x80000008 = 0x%08x, " , eax);
    word max_linear_addr_width = (eax >> 8) & 0xFF;
    
    debug::out::printf("max_linear_addr_width = %d\n" , max_linear_addr_width);
    
    // 5-level paging is supported
    if(max_linear_addr_width == 57) {
        // do whatever
    }
}

__attribute__ ((naked)) void  set_cr4_reg(qword cr4) {
    __asm__ ("mov cr4 , rdi");
    __asm__ ("ret");
}
__attribute__ ((naked)) qword fetch_cr4_reg() {
    __asm__ ("mov rax , cr4");
    __asm__ ("ret");
}