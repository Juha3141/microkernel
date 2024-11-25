LOADER_CC = x86_64-elf-gcc
LOADER_LD = x86_64-elf-ld
LOADER_AS = nasm

IMGMAKER = ./imgmaker
OBJCOPY = x86_64-elf-objcopy

LOADER_CCOPTIONS = -m32 -Os -masm=intel -fno-builtin -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -W -Wall -fpack-struct=1 -I$(HEADERSFOLDER) -fno-stack-protector
LOADER_LDOPTIONS = -m elf_i386
LOADER_ASOPTIONS = -f elf32
LOADER_LINKERSCRIPT = linker.ld
LOADER_LD_OUT = $(BINARYFOLDER)/Loader.elf

C_EXTENSION = c
AS_EXTENSION = asm

SOURCESFOLDER = src
INCLUDEPATHS = include

LINK_OBJECTS = yes

KERNEL_LOADER = Loader.bin
TARGET = $(TARGET_DIR)/OS.img

remove_target:
	rm -rf $(TARGET)

build_bootloader:
	$(LOADER_AS) bootloader.asm -f bin -o $(BINARYFOLDER)/bootloader.bin

build_final_image:
	$(OBJCOPY) -O binary $(LOADER_LD_OUT) $(BINARYFOLDER)/$(KERNEL_LOADER)

	$(IMGMAKER) $(TARGET) $(BINARYFOLDER)/bootloader.bin $(BINARYFOLDER)/$(KERNEL_LOADER) $(ROOTDIR)/$(KERNEL_IMG_LOCATION)/$(KERNEL_IMG)

run_os:
	qemu-system-x86_64 -hda $(TARGET) -m 2048 -M pc -monitor stdio

debug_run_os:
	qemu-system-x86_64 -hda $(TARGET) -m 2048 -M pc -monitor stdio -S -s

include ../../build_loader_common.mk