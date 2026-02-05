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
RAMDISK_IMG = ramdisk.img
KERNEL_IMG_LOCATION = $(ROOTBINARYFOLDER)

BUILD_KERNEL_IMG_MAKEFILE = build_kernel_img.mk
ARCH_CONFIGURATION_MAKEFILE = $(ARCHFOLDER)/$(ARCH)/arch_configurations.mk

KERNEL_AS = $(AS)
KERNEL_CC = clang++
KERNEL_LD = clang++
KERNEL_OBJDUMP = $(OBJDUMP)
KERNEL_OBJCOPY = $(OBJCOPY)
KERNEL_LINKERSCRIPT = kernel_linker.ldX

PWD = $(shell pwd)
ARCH_CONFIGURATION_FILE_LOC     = $(ARCHFOLDER)/$(ARCH)
ARCH_CONFIGURATION_FILE         = arch_configurations.hpp

define convert_ldx_to_ld
	$(KERNEL_CC) -E -P -xc $(1) $(3) > $(2)
endef