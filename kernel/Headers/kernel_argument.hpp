#ifndef _KERNEL_ARGUMENT_HPP_
#define _KERNEL_ARGUMENT_HPP_

#define KERNELINFO_STRUCTURE_SIGNATURE 0xC001D00D

#define KERNELSTRUCTURE_LENGTH  2*1024*1024 // 2MB

#define MEMORYMAP_USABLE   		1
#define MEMORYMAP_RESERVED 		2
#define MEMORYMAP_ACPI_RECLAIM 	3
#define MEMORYMAP_ACPI_NVS     	4
#define MEMORYMAP_BAD_MEMORY   	5

struct MemoryMap {
	unsigned int addr_low , addr_high;
	unsigned int length_low , length_high;
	unsigned int type;
};

struct KernelInfoStructure {
	unsigned int signature;               // 0 
	unsigned int kernel_address;          // 4
    unsigned int kernel_size;             // 8
	unsigned int memmap_count;            // 12
	unsigned int memmap_ptr;              // 16
	unsigned int kernel_stack_location;   // 20
	unsigned int kernel_stack_size;       // 24
	unsigned int pml4t_entry_location;    // 28
};

#endif