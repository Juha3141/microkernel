#ifndef _KERNEL_ARGUMENT_HPP_
#define _KERNEL_ARGUMENT_HPP_

#define MEMORYMAP_USABLE   		1
#define MEMORYMAP_RESERVED 		2
#define MEMORYMAP_ACPI_RECLAIM 	3
#define MEMORYMAP_ACPI_NVS     	4
#define MEMORYMAP_UNUSABLE      5
// EFI dedicated
#define MEMORYMAP_EFI_LOADER       6
#define MEMORYMAP_EFI_RUNTIME      7
#define MEMORYMAP_EFI_BOOT_SERVICE 8
// Video memory
#define MEMORYMAP_VIDEOMEM         9
#define MEMORYMAP_KERNEL_IMAGE     10
#define MEMORYMAP_KERNEL_STACK     11
#define MEMORYMAP_LOADER_ARGUMENT  12
#define MEMORYMAP_KSTRUCT_POOL     13
#define MEMORYMAP_KERNEL_PT_SPACE  14
#define MEMORYMAP_MISCELLANEOUS    15

#define LOADER_ARGUMENT_SIGNATURE 0x31415926
#define LOADER_ARGUMENT_LENGTH    (((unsigned long long)(sizeof(struct LoaderArgument)/4096)+(sizeof(struct LoaderArgument)%4096 ? 1 : 0))*4096)    // 500kB

#define LOADER_ARGUMENT_VIDEOMODE_NA       0x00 // hmm...
#define LOADER_ARGUMENT_VIDEOMODE_TEXTMODE 0x01
#define LOADER_ARGUMENT_VIDEOMODE_GRAPHIC  0x02
#define LOADER_ARGUMENT_VIDEOMODE_MIXED    \
	(LOADER_ARGUMENT_VIDEOMODE_TEXTMODE|LOADER_ARGUMENT_VIDEOMODE_GRAPHIC) // Text mode + Graphic

struct LoaderMemoryMap {
	unsigned int addr_low , addr_high;     // For 32-bit compatibility
	unsigned int length_low , length_high;
	unsigned int type;
};

// To-do : Remove pml4 entry field
struct __attribute__ ((packed)) LoaderArgument {
	unsigned int signature;                /* 0, always 0xC001D00D */
	unsigned int kernel_physical_location; /* 4 */
    unsigned int kernel_size;              /* 8 */
	
    unsigned int memmap_count;             /* 12 */
	// memory map passed by loader argument is an array of LoaderMemoryMap
	unsigned int memmap_location;          /* 16 */

	unsigned int kernel_stack_location;    /* 20 */
	unsigned int kernel_stack_size;        /* 24 */

	/* kstruct_mem_location: memory pool given for kstruct memory allocator
	kstruct: temporary memory allocator used before initializing the proper heap system */
	unsigned int kstruct_mem_location;     /* 28 */ 
	unsigned int kstruct_mem_size;         /* 32 */

	unsigned int loader_argument_location; /* 36 */
	unsigned int loader_argument_size;     /* 40 */

    unsigned char video_mode;              // LOADER_ARGUMENT_VIDEOMODE

    // text framebuffer(if available)
	unsigned int dbg_text_framebuffer_start;
	unsigned int dbg_text_framebuffer_end;
	unsigned int dbg_text_framebuffer_width;
	unsigned int dbg_text_framebuffer_height;
    unsigned int dbg_text_framebuffer_depth;

    // graphic framebuffer(if available)
	unsigned int dbg_graphic_framebuffer_start;
	unsigned int dbg_graphic_framebuffer_end;
    unsigned int dbg_graphic_framebuffer_width;
    unsigned int dbg_graphic_framebuffer_height;
    unsigned int dbg_graphic_framebuffer_depth;

	// miscellaneous debug interface
	unsigned int dbg_miscellaneous_ptr;

    char debug_interface_identifier[24]; // interface identification name... can be the name of the driver

	// graphics related(more detailed graphic device info)
    unsigned int graphic_device_info_size; 
    unsigned int graphic_device_info_ptr;  

	// ramdisk related
    unsigned char is_ramdisk_available; // 0: Not available, 1: Available
	unsigned int ramdisk_location;
	unsigned int ramdisk_size;
};

#endif