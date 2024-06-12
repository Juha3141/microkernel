ROOTDIR = ../..

include $(ROOTDIR)/global_variables.mk

SOURCESFOLDER = src
HEADERSFOLDER = include
ISOFOLDER     = iso
BINARYFOLDER  = $(ROOTDIR)/$(ROOTBINARYFOLDER)/$(LOADERFOLDER)/$(PWD)

BOOTLOADER = bootloader.asm

TARGET = $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)/OS.img
KERNEL_LOADER_ELF = Loader.elf
KERNEL_LOADER = Loader.bin
LINKERSCRIPT  = linker.ld

CC = x86_64-linux-gnu-gcc
LD = x86_64-linux-gnu-ld
IMGMAKER = ./imgmaker
OBJCOPY = x86_64-linux-gnu-objcopy
ASM = nasm

CCOPTIONS = -Os -masm=intel -fno-builtin -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -W -Wall -fpack-struct=1 -I$(HEADERSFOLDER) -fno-stack-protector
LDOPTIONS = -m elf_i386

MAINSOURCES = $(notdir $(subst .c,.obj,$(wildcard $(SOURCESFOLDER)/*.c)))
ASMSOURCES = $(notdir $(subst .asm,.aobj,$(wildcard $(SOURCESFOLDER)/*.asm)))
FIRSTPRIORITYOBJ = $(BINARYFOLDER)/entry.aobj

all: prepare BuildBootLoader $(MAINSOURCES) $(ASMSOURCES) BuildImage

prepare: 
	mkdir $(BINARYFOLDER)
	mkdir $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)

clean:
	rm -rf $(BINARYFOLDER)
	rm -rf $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)

BuildBootLoader:
	nasm $(BOOTLOADER) -f bin -o $(BINARYFOLDER)/bootloader.bin

BuildImage:
	$(LD) $(LDOPTIONS) -T $(LINKERSCRIPT) -o $(BINARYFOLDER)/$(KERNEL_LOADER_ELF) $(FIRSTPRIORITYOBJ) $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(BINARYFOLDER)/*obj)) -nostdlib
	$(OBJCOPY) -O binary $(BINARYFOLDER)/$(KERNEL_LOADER_ELF) $(BINARYFOLDER)/$(KERNEL_LOADER)

	$(IMGMAKER) $(TARGET) $(BINARYFOLDER)/bootloader.bin $(BINARYFOLDER)/$(KERNEL_LOADER) $(ROOTDIR)/$(KERNEL_FINAL)

run:
	qemu-system-x86_64 -hda $(TARGET) -m 2048 -M pc -monitor stdio

debugrun:
	qemu-system-x86_64 -hda $(TARGET) -m 2048 -M pc -monitor stdio -S -s

%.obj: $(SOURCESFOLDER)/%.c
	$(CC) -m32 -c $< -o $(BINARYFOLDER)/$@ $(CCOPTIONS)

%.aobj: $(SOURCESFOLDER)/%.asm
	$(ASM) $< -f elf32 -o $(BINARYFOLDER)/$@

.PHONY: clean all
