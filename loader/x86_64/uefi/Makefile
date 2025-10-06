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
	dd if=/dev/zero of=$(TARGET) bs=512 count=93750
	parted $(TARGET) -s -a minimal mklabel gpt
	parted $(TARGET) -s -a minimal mkpart EFI FAT16 2048s 93716s
	parted $(TARGET) -s -a minimal toggle 1 boot

	touch $(ROOTDIR)/memmap.txt
	dd if=/dev/zero of=$(BINARYFOLDER)/part.img bs=512 count=91669
	mformat -i $(BINARYFOLDER)/part.img -h 32 -t 32 -n 64 -c 1
	mmd -i $(BINARYFOLDER)/part.img ::/EFI
	mmd -i $(BINARYFOLDER)/part.img ::/EFI/BOOT
	mcopy -i $(BINARYFOLDER)/part.img $(EFI_TARGET) ::/EFI/BOOT
	mcopy -i $(BINARYFOLDER)/part.img $(ROOTDIR)/$(KERNEL_IMG_LOCATION)/$(KERNEL_IMG) ::/$(KERNEL_IMG) 
	mcopy -i $(BINARYFOLDER)/part.img $(ROOTDIR)/$(RAMDISK_IMG) ::/$(RAMDISK_IMG)
	

	dd if=$(BINARYFOLDER)/part.img of=$(TARGET) bs=512 count=91669 seek=2048 conv=notrunc

	rm -rf $(BINARYFOLDER)/part.img

	qemu-img convert -f raw -O vdi $(TARGET) $(subst .img,.vdi,$(TARGET))
	VBoxManage internalcommands sethduuid $(subst .img,.vdi,$(TARGET)) {b64a9a6a-279f-42e3-bc5c-8d298037c5e5}

run_os:
	qemu-system-x86_64 -cpu qemu64 -m 2048 -drive if=pflash,format=raw,unit=0,file=/usr/share/ovmf/OVMF.fd,readonly=on -drive file=$(TARGET),if=ide -monitor stdio 

debug_run_os:
	qemu-system-x86_64 -cpu qemu64 -m 2048 -drive if=pflash,format=raw,unit=0,file=/usr/share/ovmf/OVMF.fd,readonly=on -drive file=$(TARGET),if=ide -monitor stdio -S -s

include $(ROOTDIR)/build_loader_common.mk