ROOTDIR = ../..

include $(ROOTDIR)/global_variables.mk

SOURCESFOLDER = src
HEADERSFOLDER = include
ISOFOLDER     = iso
BINARYFOLDER  = $(ROOTDIR)/$(ROOTBINARYFOLDER)/$(LOADERFOLDER)/$(PWD)

TARGET = $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)/OS.iso
LOADERTARGET = Loader.elf
LINKERSCRIPT  = linker.ld

CC = x86_64-linux-gnu-gcc
LD = x86_64-linux-gnu-ld
OBJCOPY = x86_64-linux-gnu-objcopy
ASM = nasm

CCOPTIONS = -Os -masm=intel -fno-builtin -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -W -Wall -fpack-struct=1 -I$(HEADERSFOLDER) -fno-stack-protector
LDOPTIONS = -m elf_i386

MAINSOURCES = $(notdir $(subst .c,.obj,$(wildcard $(SOURCESFOLDER)/*.c)))
ASMSOURCES = $(notdir $(subst .asm,.aobj,$(wildcard $(SOURCESFOLDER)/*.asm)))
FIRSTPRIORITYOBJ = $(BINARYFOLDER)/entry.aobj

all: prepare BuildSources BuildIso

prepare: 
	mkdir $(BINARYFOLDER)
	mkdir $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)

clean:
	rm -rf $(BINARYFOLDER)
	rm -rf $(ROOTDIR)/$(FINALIMAGEFOLDER)/$(PWD)
	
BuildSources: $(MAINSOURCES) $(ASMSOURCES)
	$(LD) $(LDOPTIONS) -T $(LINKERSCRIPT) -o $(BINARYFOLDER)/$(LOADERTARGET) $(FIRSTPRIORITYOBJ) $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(BINARYFOLDER)/*obj)) -nostdlib

BuildIso:
# copy loader to the folder
	cp $(BINARYFOLDER)/$(LOADERTARGET) $(ISOFOLDER)/boot/$(LOADERTARGET)
	cp $(ROOTDIR)/$(KERNEL_FINAL) $(ISOFOLDER)
	grub-mkrescue --output=$(TARGET) $(ISOFOLDER)/

run:
	qemu-system-x86_64 -cdrom $(TARGET) -hda rdimg-test.img -boot d -m 2048 -M pc -rtc base=localtime -monitor stdio

debugrun:
	qemu-system-x86_64 -cdrom $(TARGET) -hda rdimg-test.img -boot d -m 2048 -M pc -rtc base=localtime -monitor stdio -S -s

%.obj: $(SOURCESFOLDER)/%.c
	$(CC) -m32 -c $< -o $(BINARYFOLDER)/$@ $(CCOPTIONS)

%.aobj: $(SOURCESFOLDER)/%.asm
	$(ASM) $< -f elf32 -o $(BINARYFOLDER)/$@

.PHONY: clean all