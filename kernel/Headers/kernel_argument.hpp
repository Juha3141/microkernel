#ifndef _KERNEL_ARGUMENT_HPP_
#define _KERNEL_ARGUMENT_HPP_

#define KERNELINFO_STRUCTURE_SIGNATURE 0xC001D00D

#define KERNELSTRUCTURE_LENGTH  2*1024*1024 // 2MB

#define MEMORYMAP_USABLE   		1
#define MEMORYMAP_RESERVED 		2
#define MEMORYMAP_ACPI_RECLAIM 	3
#define MEMORYMAP_ACPI_NVS     	4
#define MEMORYMAP_BAD_MEMORY   	5

#define KERNELARG_VIDEOMODE_TEXTMODE 0x00
#define KERNELARG_VIDEOMODE_GRAPHIC  0x01

struct MemoryMap {
	unsigned int addr_low , addr_high;
	unsigned int length_low , length_high;
	unsigned int type;
};

// To-do : Remove pml4 entry field
struct KernelArgument {
	unsigned int signature;               // 0 
	unsigned int kernel_address;          // 4
    unsigned int kernel_size;             // 8
	unsigned int memmap_count;            // 12
	unsigned int memmap_ptr;              // 16
	unsigned int kernel_stack_location;   // 20
	unsigned int kernel_stack_size;       // 24
	unsigned int pml4t_entry_location;    // 28
	unsigned int pml4_entry_size; 		  // 32

	unsigned int total_kernel_area_start; // 36
	unsigned int total_kernel_area_end;   // 40

	unsigned int kernel_linear_address;   // 44

	// graphics related
	unsigned char video_mode;

	unsigned int framebuffer_addr;
	unsigned int framebuffer_width;
	unsigned int framebuffer_height;

	unsigned int framebuffer_depth;
};

#endif