#ifndef _KERNEL_ARGUMENT_HPP_
#define _KERNEL_ARGUMENT_HPP_

#define MEMORYMAP_USABLE   		1
#define MEMORYMAP_RESERVED 		2
#define MEMORYMAP_ACPI_RECLAIM 	3
#define MEMORYMAP_ACPI_NVS     	4
#define MEMORYMAP_EFI_RUNTIME   5
#define MEMORYMAP_BAD_MEMORY   	6

#define LOADER_ARGUMENT_SIGNATURE 0xC001D00D
#define LOADER_ARGUMENT_LENGTH    2*1024*1024 // 2MB

#define LOADER_ARGUMENT_VIDEOMODE_NA       0x00 // hmm...
#define LOADER_ARGUMENT_VIDEOMODE_TEXTMODE 0x01
#define LOADER_ARGUMENT_VIDEOMODE_GRAPHIC  0x02
#define LOADER_ARGUMENT_VIDEOMODE_MIXED    0x03 // Text mode + Graphic

struct MemoryMap {
	unsigned int addr_low , addr_high;     // For 32-bit compatibility
	unsigned int length_low , length_high;
	unsigned int type;
};

// To-do : Remove pml4 entry field
struct LoaderArgument {
	unsigned int signature;               // 0, always 0xC001D00D
	unsigned int kernel_address;          // 4
    unsigned int kernel_size;             // 8
	
    unsigned int memmap_count;            // 12
	unsigned int memmap_ptr;              // 16

	unsigned int kernel_stack_location;   // 20
	unsigned int kernel_stack_size;       // 24

	unsigned int total_kernel_area_start; // 28
	unsigned int total_kernel_area_end;   // 32

	unsigned int kernel_linear_address;   // 36

    unsigned char video_mode;           // LOADER_ARGUMENT_VIDEOMODE

    // text framebuffer(if available)
	unsigned int dbg_text_framebuffer_addr;
	unsigned int dbg_text_framebuffer_width;
	unsigned int dbg_text_framebuffer_height;
    unsigned int dbg_text_framebuffer_depth;

    // graphic framebuffer(if available)
    unsigned int dbg_graphic_framebuffer_addr;
    unsigned int dbg_graphic_framebuffer_width;
    unsigned int dbg_graphic_framebuffer_height;
    unsigned int dbg_graphic_framebuffer_depth;

    char debug_interface_identifier[24]; // interface identification name... can be the name of the driver

	// graphics related(more detailed graphic device info)
    unsigned int graphic_device_info_size; 
    unsigned int graphic_device_info_ptr;  

	// ramdisk related
    unsigned char is_ramdisk_available; // 0: Not available, 1: Available
	unsigned int ramdisk_address;
	unsigned int ramdisk_size;
};

#endif