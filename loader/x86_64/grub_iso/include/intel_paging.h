#ifndef _PAGING_H_
#define _PAGING_H_

#include <../../../../kernel/include/loader/loader_argument.hpp>

#define PAGE_SIZE                0x200000

#define PAGE_MAX_ENTRY_COUNT     512
#define PAGE_PML4ENTRY_FLAGS_P   0b000000000001
#define PAGE_PML4ENTRY_FLAGS_RW  0b000000000010
#define PAGE_PML4ENTRY_FLAGS_US  0b000000000100
#define PAGE_PML4ENTRY_FLAGS_PWT 0b000000001000
#define PAGE_PML4ENTRY_FLAGS_PCD 0b000000010000
#define PAGE_PML4ENTRY_FLAGS_A   0b000000100000
#define PAGE_PML4ENTRY_FLAGS_EXB 0b100000000000

#define PAGE_PDPTENTRY_FLAGS_P   0b00000000000001
#define PAGE_PDPTENTRY_FLAGS_RW  0b00000000000010
#define PAGE_PDPTENTRY_FLAGS_US  0b00000000000100
#define PAGE_PDPTENTRY_FLAGS_PWT 0b00000000001000
#define PAGE_PDPTENTRY_FLAGS_PCD 0b00000000010000
#define PAGE_PDPTENTRY_FLAGS_A   0b00000000100000
#define PAGE_PDPTENTRY_FLAGS_D   0b00000001000000 // Ignored when PDPTE references Page Directory
#define PAGE_PDPTENTRY_FLAGS_PS  0b00000010000000 // Page size, 1 = 1GB page, 0 = 2MB or 4KB page.
#define PAGE_PDPTENTRY_FLAGS_G   0b00000100000000 // 
#define PAGE_PDPTENTRY_FLAGS_PAT 0b01000000000000
#define PAGE_PDPTENTRY_FLAGS_EXB 0b10000000000000

#define PAGE_PDENTRY_FLAGS_P     0b00000000000001
#define PAGE_PDENTRY_FLAGS_RW    0b00000000000010
#define PAGE_PDENTRY_FLAGS_US    0b00000000000100
#define PAGE_PDENTRY_FLAGS_PWT   0b00000000001000
#define PAGE_PDENTRY_FLAGS_PCD   0b00000000010000
#define PAGE_PDENTRY_FLAGS_A     0b00000000100000
#define PAGE_PDENTRY_FLAGS_D     0b00000001000000
#define PAGE_PDENTRY_FLAGS_PS    0b00000010000000
#define PAGE_PDENTRY_FLAGS_G     0b00000100000000
#define PAGE_PDENTRY_FLAGS_EXB   0b10000000000000

#define PAGE_GET_PML4ENTRY_NUM(addr)  (((addr) >> 39) & 0x1FF)
#define PAGE_GET_PDPTENTRY_NUM(addr)  (((addr) >> 30) & 0x1FF)
#define PAGE_GET_PDENTRY_NUM(addr)    (((addr) >> 21) & 0x1FF)
#define PAGE_GET_OFFSET(addr)         ((addr) & 0x1FFFFF) 

struct PageEntry {
    unsigned int BaseAddressLow_Flags;  // Total 52bit, Note that base address should be aligned to 4KB
    unsigned int BaseAddressHigh;
};

void SetPageEntry(struct PageEntry *PageEntry , unsigned int BaseAddressLow , unsigned int BaseAddressHigh , unsigned short Flags);
unsigned int SetupPML4_custom(unsigned int start_address , struct MemoryMap *memmap);
void ModifyPML4Entry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int base_low , unsigned int base_high , unsigned short flags);
void ModifyPDPTEntry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int pdpt_entry , unsigned int base_low , unsigned int base_high , unsigned short flags);
void ModifyPDEntry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int pdpt_entry , unsigned int pd_entry , unsigned int base_low , unsigned int base_high , unsigned short flags);

void RelocatePage(unsigned int kernel_address , unsigned int kernel_page_size , unsigned int new_address , unsigned int pml4_entry_address , unsigned short new_flags);

#endif