GNU_EFI_DIR = $(ROOTDIR)/gnu-efi

LOADER_CC = gcc
LOADER_LD = ld
LOADER_AS = nasm

LOADER_CCOPTIONS = -g -masm=intel -fpack-struct=1 -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
LOADER_LDOPTIONS = -shared -Bsymbolic -L$(GNU_EFI_DIR)/x86_64/lib -L$(GNU_EFI_DIR)/x86_64/gnuefi $(GNU_EFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o 
LOADER_ASOPTIONS = -f elf64

LOADER_LD_LIBRARIES = -lgnuefi -lefi
LOADER_LINKERSCRIPT = $(GNU_EFI_DIR)/gnuefi/elf_x86_64_efi.lds
LOADER_LD_OUT = $(BINARYFOLDER)/uefi.so

C_EXTENSION  = c
AS_EXTENSION = asm

SOURCESFOLDER = src
INCLUDEFOLDER = include

INCLUDEPATHS = $(GNU_EFI_DIR)/inc

LINK_OBJECTS = yes

EFI_TARGET = $(BINARYFOLDER)/bootx64.efi
TARGET = $(TARGET_DIR)/uefi.img

remove_target:

build_bootloader:

build_efi_executable:
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(LOADER_LD_OUT) $(EFI_TARGET)

build_final_image: build_efi_executable
	dd if=$(ROOTDIR)/emptygpt.img of=$(BINARYFOLDER)/img_header.img count=2048

	cp $(ROOTDIR)/tempvol.img $(BINARYFOLDER)/part.img
	mmd -i $(BINARYFOLDER)/part.img ::/EFI
	mmd -i $(BINARYFOLDER)/part.img ::/EFI/BOOT
	mcopy -i $(BINARYFOLDER)/part.img $(EFI_TARGET) ::/EFI/BOOT
	mcopy -i $(BINARYFOLDER)/part.img $(ROOTDIR)/$(KERNEL_IMG_LOCATION)/$(KERNEL_IMG) ::/$(KERNEL_IMG)

	if [ -f $(ROOTDIR)/$(RAMDISK_IMG) ]; then \
		mcopy -i $(BINARYFOLDER)/part.img $(ROOTDIR)/$(RAMDISK_IMG) ::/$(RAMDISK_IMG); \
	fi

	cat $(BINARYFOLDER)/img_header.img $(BINARYFOLDER)/part.img > $(TARGET)

	qemu-img convert -f raw -O vdi $(TARGET) $(subst .img,.vdi,$(TARGET))

run_os:
	qemu-system-x86_64 -m 8192 -drive if=pflash,format=raw,unit=0,file=/usr/share/ovmf/OVMF.fd,readonly=on -drive file=$(TARGET),if=ide -monitor stdio -cpu qemu64,+la57,phys-bits=52,host-phys-bits=off -no-reboot -no-shutdown

debug_run_os:
	qemu-system-x86_64 -m 8192 -drive if=pflash,format=raw,unit=0,file=/usr/share/ovmf/OVMF.fd,readonly=on -drive file=$(TARGET),if=ide -monitor stdio -S -s -cpu qemu64,+la57,phys-bits=52,host-phys-bits=off -no-reboot -no-shutdown

include $(ROOTDIR)/build_loader_common.mk