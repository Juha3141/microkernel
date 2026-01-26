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

typedef unsigned long x86_page_entry_t;

struct PageTableData ARCHDEP {
    x86_page_entry_t *cr3_base = nullptr;
    bool pml5_enabled = false;
};

extern "C" void enable_5_level_paging(void);

__attribute__ ((naked)) void  set_cr3_reg(qword cr3);
__attribute__ ((naked)) qword  fetch_cr3_reg();
__attribute__ ((naked)) void  set_cr4_reg(qword cr4);
__attribute__ ((naked)) qword fetch_cr4_reg();

#endif