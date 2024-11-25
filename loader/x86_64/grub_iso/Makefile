ISOFOLDER     = iso

LOADER_CC = x86_64-linux-gnu-gcc
LOADER_LD = x86_64-linux-gnu-ld
LOADER_AS = nasm

LOADER_CCOPTIONS = -m32 -Os -masm=intel -fno-builtin -fno-stack-protector -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -W -Wall -fpack-struct=1 -I$(HEADERSFOLDER) -fno-stack-protector
LOADER_ASOPTIONS = -f elf32
LOADER_LDOPTIONS = -m elf_i386
LOADER_LINKERSCRIPT = linker.ld
LOADER_LD_OUT = $(BINARYFOLDER)/Loader.elf

C_EXTENSION   = c
AS_EXTENSION  = asm

SOURCESFOLDER = src
INCLUDEPATHS  = include

LINK_OBJECTS = yes

TARGET = $(TARGET_DIR)/OS.iso

remove_target:
	rm -rf $(TARGET_DIR)/$(TARGET)

build_bootloader:
	

build_final_image:
	cp $(LOADER_LD_OUT) $(ISOFOLDER)/boot/
	cp $(ROOTDIR)/$(KERNEL_IMG_LOCATION)/$(KERNEL_IMG) $(ISOFOLDER)/
	grub-mkrescue --output=$(TARGET) $(ISOFOLDER)/

run_os: $(TARGET)
	qemu-system-x86_64 -cdrom $(TARGET) -hda rdimg-test.img -boot d -m 2048 -M pc -rtc base=localtime -monitor stdio

debug_run_os: $(TARGET)
	qemu-system-x86_64 -cdrom $(TARGET) -hda rdimg-test.img -boot d -m 2048 -M pc -rtc base=localtime -monitor stdio

include ../../build_loader_common.mk