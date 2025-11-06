#ifndef _PAGE_TABLE_HPP_
#define _PAGE_TABLE_HPP_

#include <kernel/essentials.hpp>

#define PML4_FLAGS_PRESENT  0x01
#define PML4_FLAGS_RW       0x02
#define PML4_FLAGS_US       0x04
#define PML4_FLAGS_PWT      0x08
#define PML4_FLAGS_PCD      0x10
#define PML4_FLAGS_ACCESSED 0x20
#define PML4_FLAGS_DIRTY    0x40
#define PML4_FLAGS_PS       0x80
#define PML4_FLAGS_GLOBAL   0x100
#define PML4_FLAGS_PAT      0x400
#define PML4_ENTRY_MAXCOUNT 512

#define PML4_FLAGS_EXD      0x8000000000000000

typedef unsigned long pml4_entry_t , pdpt_entry_t , pdentry_t , ptentry_t;

struct PageTableData ARCHDEP {
    max_t base_location;
    
    max_t pml4_entry_max_offset = 0;
    pml4_entry_t *pml4_entry = nullptr;
    
    max_t pdpt_entry_max_offset = 0;
    pdpt_entry_t *pdpt_entry = nullptr;
    
    max_t pd_entry_max_offset = 0;
    pdentry_t    *pd_entry = nullptr;
    
    max_t pt_entry_max_offset = 0;
    ptentry_t    *pt_entry = nullptr;
};

extern "C" void enable_5_level_paging(void);

qword set_pml4_entry(qword physical_address , dword flags);
__attribute__ ((naked)) void  set_cr4_reg(qword cr4);
__attribute__ ((naked)) qword fetch_cr4_reg();

#endif