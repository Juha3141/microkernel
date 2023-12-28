 #include <intel_paging.h>

void SetPageEntry(struct PageEntry *PageEntry , unsigned int BaseAddressLow , unsigned int BaseAddressHigh , unsigned short Flags) {
    PageEntry->BaseAddressLow_Flags = (BaseAddressLow & 0xFFFFF000)|(Flags & 0xFFF);
    PageEntry->BaseAddressHigh = BaseAddressHigh;
}

unsigned int SetupPML4_custom(unsigned int start_address , struct MemoryMap *memmap) {
    // The entries exist linearly (adjacent to each other)
    // PML4 -> PDPT -> PD, 512 entries each
    // one PML4 consists 512 PDPT entry, one PDPT entry consists 512 PD entry
    
    // Total size : PML4(512*8) + PDPT(512*8*1) + PD(512*512*8*1) = 0x202000 (approx. 2MB)
    
    struct PageEntry *pml4_entry = (struct PageEntry *)start_address; // total 512 entries
    struct PageEntry *pdpt_entry = pml4_entry+(PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry)); // total 512 entries
    struct PageEntry *pde_entry = pdpt_entry+(PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry));

    unsigned int physicaladdr_low = 0;
    unsigned int physicaladdr_high = 0;
    for(int i = 0; i < PAGE_MAX_ENTRY_COUNT; i++) {
        SetPageEntry(&(pml4_entry[i]) , 0 , 0 , 0);
    }
    unsigned int k = 1;
    SetPageEntry(&(pml4_entry[0]) , pdpt_entry , 0x00 , PAGE_PML4ENTRY_FLAGS_P|PAGE_PML4ENTRY_FLAGS_RW);
    for(unsigned int i = 0 , j = 0; i < 32; i++) {
        SetPageEntry(&(pdpt_entry[i]) , pde_entry , 0x00 , PAGE_PDPTENTRY_FLAGS_P|PAGE_PDPTENTRY_FLAGS_RW);
        
        for(unsigned int j = 0; j < PAGE_MAX_ENTRY_COUNT; j++) {
            SetPageEntry(&(pde_entry[j]) , ((unsigned int)j << 21)|(((unsigned int)i & 0b11) << 30) , ((unsigned int)i >> 2) , PAGE_PDENTRY_FLAGS_P|PAGE_PDENTRY_FLAGS_RW|PAGE_PDENTRY_FLAGS_PS);
            /*if(((j << 21)|((i & 0b11) << 30)) == 0xC0000000) {
                SetPageEntry(&(pde_entry[j]) , 0xC0000000 , k , PAGE_PDENTRY_FLAGS_P|PAGE_PDENTRY_FLAGS_RW|PAGE_PDENTRY_FLAGS_PS);
                k += 1;
            }*/
        }
        pde_entry += PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry);
    }
    return (unsigned int)pde_entry;
}

void ModifyPML4Entry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int base_low , unsigned int base_high , unsigned short flags) {
    unsigned int pml4_entry_address = entry_addr; // total 512 entries
    struct PageEntry *t_pml4_entry = (struct PageEntry *)pml4_entry_address;

    SetPageEntry(&(t_pml4_entry[pml4_entry]) , base_low , base_high , flags);
}

void ModifyPDPTEntry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int pdpt_entry , unsigned int base_low , unsigned int base_high , unsigned short flags) {
    unsigned int pml4_entry_address = entry_addr; // total 512 entries
    struct PageEntry *t_pml4_entry = (struct PageEntry *)pml4_entry_address;
    struct PageEntry *t_pdpt_entry = (struct PageEntry *)(t_pml4_entry[pml4_entry].BaseAddressLow_Flags & 0xFFFFF000);
    
    SetPageEntry(&(t_pdpt_entry[pdpt_entry]) , base_low , base_high , flags);
}

void ModifyPDEntry(unsigned int entry_addr , unsigned int pml4_entry , unsigned int pdpt_entry , unsigned int pd_entry , unsigned int base_low , unsigned int base_high , unsigned short flags) {
    unsigned int pml4_entry_address = entry_addr; // total 512 entries
    struct PageEntry *t_pml4_entry = (struct PageEntry *)pml4_entry_address;
    struct PageEntry *t_pdpt_entry = (struct PageEntry *)(t_pml4_entry[pml4_entry].BaseAddressLow_Flags & 0xFFFFF000);
    struct PageEntry *t_pd_entry = (struct PageEntry *)(t_pdpt_entry[pdpt_entry].BaseAddressLow_Flags & 0xFFFFF000);

    SetPageEntry(&(t_pd_entry[pd_entry]) , base_low , base_high , flags);
}

void RelocatePage(unsigned int kernel_address , unsigned int kernel_page_size , unsigned int new_address , unsigned int pml4_entry_address , unsigned short new_flags) {
    unsigned int pml4e_index = PAGE_GET_PML4ENTRY_NUM(new_address);
    unsigned int pdpte_index = PAGE_GET_PDPTENTRY_NUM(new_address);
    unsigned int pde_index = PAGE_GET_PDENTRY_NUM(new_address);
    unsigned int offset = PAGE_GET_OFFSET(new_address);
    PrintString(0x07 , "Setting base address of page... ");
    PrintString(0x07 , "pml4 index : %d | pdpt index : %d | pd_index : %d\n" , pml4e_index , pdpte_index , pde_index);
    PrintString(0x07 , "new base address : 0x%X\n" , kernel_address);
    for(unsigned int i = 0; i < kernel_page_size; i++) {
        if(pde_index >= 511) {
            pde_index = 0;
            pdpte_index += 1;
        } 
        ModifyPDEntry(pml4_entry_address , pml4e_index , pdpte_index , pde_index , kernel_address+(PAGE_SIZE*i) , 0x00 , new_flags);
        pde_index += 1;
    }
}