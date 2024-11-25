KERNELFOLDER  = kernel
ARCHFOLDER    = arch
INTEGRATEDFOLDER = integrated
FILESYSTEMFOLDER = filesystem
DRIVERSFOLDER = drivers
LOADERFOLDER  = loader
FINALIMAGEFOLDER = image

ROOTBINARYFOLDER = bin

COMMON_HEADERFOLDER  = include
COMMON_SRCFOLDER  = src

KERNEL_ELF = Kernel.elf
KERNEL_IMG = Kernel.krn 
KERNEL_IMG_LOCATION = $(ROOTBINARYFOLDER)

BUILD_KERNEL_IMG_MAKEFILE = build_kernel_img.mk

KERNEL_AS = $(AS)
KERNEL_CC = $(KERNEL_COMPILER)-$(CC)
KERNEL_LD = $(KERNEL_COMPILER)-$(LD)
KERNEL_OBJDUMP = $(KERNEL_COMPILER)-$(OBJDUMP)
KERNEL_OBJCOPY = $(KERNEL_COMPILER)-$(OBJCOPY)
KERNEL_LINKERSCRIPT = kernel_linker.ld

PWD = $(shell pwd)
ARCH_CONFIGURATION_FILE_LOC     = $(ARCHFOLDER)/$(ARCH)
ARCH_CONFIGURATION_FILE         = arch_configurations.hpp

define convert_hpp_to_ld
	echo \#include \"$(notdir $(2))\" > $(dir $(2))dummy.ld
	$(KERNEL_CC) -E -P -xc -DLINKER_SCRIPT $(dir $(2))dummy.ld > $(subst .hpp,.ld,$(2))
	rm $(dir $(2))dummy.ld
endef