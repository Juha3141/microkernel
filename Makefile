include global_variables.mk

BASH = bash
QEMU = qemu-system-x86_64
QEMU_OPTION = -m 8192 -rtc base=localtime -M pc -boot c
TARGET = OS.iso

all: BuildLibrary BuildKernel BuildLoader

BuildLibrary:
	make -C $(KRNLIBRARYFOLDER) all

BuildKernel:
	make -C $(KERNELFOLDER)/$(HALFOLDER) all
	make -C $(KERNELFOLDER) all

	cp $(KERNELFOLDER)/$(KERNELTARGET) $(LOADERFOLDER)/iso/$(KERNELTARGET)

BuildLoader:
	make -C $(LOADERFOLDER) all

clean:
	make -C $(KRNLIBRARYFOLDER) clean
	make -C $(KERNELFOLDER)/$(HALFOLDER) all
	make -C $(KERNELFOLDER) clean
	make -C $(LOADERFOLDER) clean

	rm -rf $(TARGET)

run: virtualbox

qemu:
	$(QEMU) -cdrom $(TARGET) $(QEMU_OPTION)

debug: 
	$(QEMU) -cdrom $(TARGET) $(QEMU_OPTION) -s -S -serial stdio

qemu_hd_old:
	$(QEMU) -hda $(TARGET) $(QEMU_OPTION) -boot c

debug_hd_old: 
	$(QEMU) -hda $(TARGET) $(QEMU_OPTION) -boot c -s -S -serial stdio

virtualbox:
	# qemu-img convert -O qcow2 $(TARGET) $(patsubst %.img,%.qcow2,$(TARGET))
	vboxmanage startvm "OS - Embedded" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
