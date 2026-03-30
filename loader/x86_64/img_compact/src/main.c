#include <strings.h>
#include <kernel/configurations.hpp>
#include <arch/configurations.hpp>
#include <intel_paging.h>

typedef struct {
    unsigned int addr_low;
    unsigned int addr_high;

    unsigned int len_low;
    unsigned int len_high;

    unsigned int type;

    unsigned int acpi;
}e820_memmap_entry_t;

void PrintString(unsigned char color , const char *fmt , ...);

void main(unsigned int memmap_addr , unsigned int memmap_size) {
    e820_memmap_entry_t *memmap = (e820_memmap_entry_t *)memmap_addr;
    for(int i = 0; i < memmap_size/sizeof(e820_memmap_entry_t); i++) {
        PrintString(0x07 , "%d\n" , memmap[i].type);
    }
    while(1) {
        ;
    }
}