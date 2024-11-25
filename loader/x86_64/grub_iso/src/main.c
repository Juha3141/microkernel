#include <multiboot.h>
#include <strings.h>
#include <stdarg.h>

#include <intel_paging.h>

#define KERNEL_STRUCTURE_SIGNATURE 0xC001D00D
#define KERNEL_STRUCTURE_STACKSIZE 8*1024*1024
#define KERNEL_NEW_HIGHER_HALF     0xC0000000

typedef struct m_mmap_x32 {
	unsigned int size;
	unsigned int addr_low , addr_high;
	unsigned int length_low , length_high;
	unsigned int type;
}multiboot_memory_map_x32_t;

int DetectMemory(struct MemoryMap *mmap , struct multiboot_info *MultibootInfo) {
	unsigned int start = MultibootInfo->mmap_addr;
	unsigned int end = MultibootInfo->mmap_addr+MultibootInfo->mmap_length;
	multiboot_memory_map_x32_t *entry = (multiboot_memory_map_x32_t *)start;

	int i = 0;
	while(1) {
		entry = (multiboot_memory_map_x32_t *)(((unsigned int)entry)+(entry->size+sizeof(entry->size)));
		if(entry >= end) break;
		mmap[i].addr_high = entry->addr_high;
		mmap[i].addr_low = entry->addr_low;
		mmap[i].length_high = entry->length_high;
		mmap[i].length_low = entry->length_low;
		mmap[i].type = entry->type;
		PrintString(0x07 , "h:%X,l:%X,h:%X,l:%X,T:%d\n" , mmap[i].addr_high , mmap[i].addr_low , mmap[i].length_high , mmap[i].length_low , mmap[i].type);
		i++;
	}
	return i;
}

unsigned int align(unsigned int address , unsigned int alignment) {
	return (((unsigned int)(address/alignment))+1)*alignment;
}

multiboot_module_t *search_module(struct multiboot_info *multiboot_info , const char *cmdline) {
	multiboot_module_t *mod;
	for(int i = 0; i < multiboot_info->mods_count; i++) {
		mod = (multiboot_module_t *)(multiboot_info->mods_addr+(i*sizeof(multiboot_module_t)));
		if(strcmp(cmdline , mod->cmdline) == 0) {
			return mod;
		}
	}
	return 0x00;
}

char kernel_command[] = "This is a kernel by the way";
char ramdisk_command[] = "This is a ramdisk by the way";

void JumpToKernel64(unsigned int address , unsigned int pml4t_entry_location);

void Main(struct multiboot_info *multiboot_info) {
	multiboot_module_t *kernel_module;
	multiboot_module_t *ramdisk_module;
	if((multiboot_info->mods_count == 0)) {
		PrintString(0x04 , "No Modules found!\n");
		while(1) {
			;
		}
	}
	PrintString(0x07 , "Modules count   : %d\n" , multiboot_info->mods_count);
	kernel_module = search_module(multiboot_info , kernel_command);
	ramdisk_module = search_module(multiboot_info , ramdisk_command);
	if(kernel_module == 0x00||ramdisk_module == 0x00) {
		PrintString(0x04 , "No Modules found!\n");
		while(1) {
			;
		}
	}
	// Search module, compare the command line
	
	// https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
	// https://wiki.osdev.org/Creating_a_64-bit_kernel_using_a_separate_loader
	
	// Relocating kernel to higher half : to 0x800000000000 // physical memory : just module location
	// Relocate Kernel in 4MB Area??
	PrintString(0x07 , "Kernel Image : 0x%X~0x%X\n" , kernel_module->mod_start , kernel_module->mod_end);
	unsigned int kernel_addr = align(kernel_module->mod_start , PAGE_SIZE);
	unsigned int kernel_size = kernel_module->mod_end-kernel_module->mod_start;

	unsigned int ramdisk_addr = ramdisk_module->mod_start;
	unsigned int ramdisk_size = ramdisk_module->mod_end-ramdisk_module->mod_start;

	unsigned int temporary_addr = kernel_addr+kernel_size;

	unsigned int pml4t_entry_location;
	unsigned int pml4t_entry_size;
	// Relocate kernel to new aligned memory address
	// (Align address to 2MB for Higher-half kernel)
	memcpy((void *)temporary_addr , (void *)kernel_module->mod_start , kernel_size); // temporary area for relocation
	memcpy((void *)kernel_addr , (void *)temporary_addr , kernel_size);
	memset((void *)temporary_addr , 0 , kernel_size); // clean temporary area

	PrintString(0x07 , "New aligned image : 0x%X~0x%X\n" , kernel_addr , kernel_addr+kernel_size);
	PrintString(0x07 , "RAMDisk location  : 0x%X~0x%X\n" , ramdisk_addr , ramdisk_addr+ramdisk_size);
	
	// align kernel end address
	unsigned int kernel_struct_addr = align(kernel_addr+kernel_size , PAGE_SIZE);
	
	// write the signature to the kernel struct area
	struct KernelArgument *kargument = (struct KernelArgument *)kernel_struct_addr;
	PrintString(0x07 , "Kernel structure : 0x%X\n" , kernel_struct_addr);
	memset(kargument , 0 , sizeof(struct KernelArgument));
	kargument->signature = KERNEL_STRUCTURE_SIGNATURE;
	// total_kernel_boundary : total boundary that contains kernel and other necessary stuff
	// physical memory manager will protect the "total bound"
	kargument->total_kernel_area_start = kernel_addr;
	
	kargument->kernel_address = kernel_addr;
	kargument->kernel_size = kernel_size;

	kargument->ramdisk_address = ramdisk_addr;
	kargument->ramdisk_size = ramdisk_size;

	kernel_struct_addr += sizeof(struct KernelArgument);
	
	// Global memory map
	struct MemoryMap *memmap = (struct MemoryMap *)kernel_struct_addr;
	kargument->memmap_ptr = (struct MemMap *)kernel_struct_addr;
	// initialize memory map area
	memset(memmap , 0 , sizeof(struct MemoryMap)*32);
	kernel_struct_addr += sizeof(struct MemoryMap)*32; // maximum 32
	
	PrintString(0x07 , "memmap location : 0x%X\n" , memmap);
	kargument->memmap_count = DetectMemory(memmap , multiboot_info);

	kargument->kernel_stack_location = kernel_struct_addr;
	kargument->kernel_stack_size = KERNEL_STRUCTURE_STACKSIZE;
	memset((unsigned char *)kargument->kernel_stack_location , 0 , kargument->kernel_stack_size);
	kernel_struct_addr += kargument->kernel_stack_size; // 128kb kernel stack
	
	PrintString(0x07 , "kernel stack address : 0x%X\n" , kargument->kernel_stack_location);
	kernel_struct_addr = align(kernel_struct_addr , 4096);
	pml4t_entry_location = kernel_struct_addr;
	PrintString(0x07 , "PML4 table location  : 0x%X\n" , kernel_struct_addr);
	unsigned int pml4t_entry_end = SetupPML4_custom(kernel_struct_addr , memmap);
	pml4t_entry_size = pml4t_entry_end-pml4t_entry_location;

	// address should be aligned to 2MB
	kargument->kernel_linear_address = KERNEL_NEW_HIGHER_HALF;
	PrintString(0x07 , "kernel_page_size : %d\n" , ((kargument->kernel_size)/PAGE_SIZE)+((kargument->kernel_size%PAGE_SIZE != 0)));
	RelocatePage(kargument->kernel_address , ((kargument->kernel_size)/PAGE_SIZE)+((kargument->kernel_size%PAGE_SIZE != 0)) , kargument->kernel_linear_address , pml4t_entry_location , PAGE_PDENTRY_FLAGS_P|PAGE_PDENTRY_FLAGS_RW|PAGE_PDENTRY_FLAGS_PS);
	// Make original kernel not present
	// RelocatePage(kargument->kernel_address , ((kargument->kernel_size)/PAGE_SIZE)+((kargument->kernel_size%PAGE_SIZE != 0)) , kargument->kernel_address , kargument->pml4t_entry_location , PAGE_PDENTRY_FLAGS_PS);

	kargument->total_kernel_area_end = pml4t_entry_end;

	// graphic system!
	kargument->video_mode = KERNELARG_VIDEOMODE_TEXTMODE;
	kargument->framebuffer_addr = 0xB8000;
	kargument->framebuffer_width = 80;
	kargument->framebuffer_height = 25;
	kargument->framebuffer_depth = 0x00;
	if((multiboot_info->flags & (1 << 12)) == (1 << 12)) {
		kargument->video_mode = KERNELARG_VIDEOMODE_GRAPHIC;
		kargument->framebuffer_addr = multiboot_info->framebuffer_addr;
		kargument->framebuffer_width = multiboot_info->framebuffer_width;
		kargument->framebuffer_height = multiboot_info->framebuffer_height;
		kargument->framebuffer_depth = multiboot_info->framebuffer_bpp;
	}

	JumpToKernel64((unsigned int)kargument , pml4t_entry_location);

	while(1) {
		;
	}
}

// structure of 'Kernel Structure'
// Start : End of Kernel
// Signature Size : 4B
// Memory Map Size : 32*20 = 640B
// Kernel Stack    : 128*1024 = 128kB
// PML4 Table Entry : Aligned to 4Kb
// +-------------+--------------+-------------------+--------------------+
// |    Kernel   |              |                   |                    |
// |  Structure  |  Memory Map  | Kernel Stack Area |  PML4 Table Entry  |
// | Information |              |                   |                    |
// +-------------+--------------+-------------------+--------------------+ 

// kinda changed the way I name the variables...

void PrintString(unsigned char color , const char *fmt , ...) {
	static int off=0;
	unsigned char *vmem = (unsigned char  *)0xB8000;
	va_list ap;
	char string[512] = {0 , };
	va_start(ap , fmt);
	vsprintf(string , fmt , ap);
	va_end(ap);
	for(int i = 0; string[i] != 0; i++) {
		switch(string[i]) {
			case '\n':
				off = ((off/80)+(off%80 != 0))*80;
				break;
			default:
				*(vmem+(off*2)) = string[i];
				*(vmem+(off*2)+1) = color;
				off++;
				break;
		}
	}
}