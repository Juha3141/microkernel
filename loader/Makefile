ROOTDIR = ..

include $(ROOTDIR)/global_variables.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)/$(LOADERFOLDER)
ISOFOLDER = iso
TARGET = OS.iso
KERNEL32BINARYFOLDER = $(BINARYFOLDER)/x32
LOADERTARGET = Loader.elf

clean:
	make -C x32 clean

all: Buildx32
# copy loader to the folder
	cp $(KERNEL32BINARYFOLDER)/$(LOADERTARGET) $(ISOFOLDER)/boot/$(LOADERTARGET)
	cp $(ROOTDIR)/$(KERNEL_FINAL) $(ISOFOLDER)
	grub-mkrescue --output=$(ROOTDIR)/$(TARGET) $(ISOFOLDER)/

Buildx32:
	make -C x32 all

