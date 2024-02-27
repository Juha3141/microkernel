include global_variables.mk
include common_compilers.mk

BASH = bash
QEMU = qemu-system-x86_64
QEMU_OPTION = -m 8192 -rtc base=localtime -M pc -boot d
TARGET = OS.iso

FIRSTPRIORITY_OBJECT = $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/main.obj
KERNEL_OBJECTS = $(filter-out $(FIRSTPRIORITY_OBJECT),$(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*.obj)) $(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*/*.obj)
ARCH_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*.obj) $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*/*.obj)
DRIVER_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(DRIVERSFOLDER)/*/*.obj)

LIBRARIES = $(patsubst %.a,%,$(subst lib,-l,$(notdir $(wildcard $(ROOTBINARYFOLDER)/$(KRNLIBRARYFOLDER)/*.a))))

LINKERSCRIPT = kernel_linker.ld

all: BuildLibrary BuildKernel BuildArchitecture BuildDrivers BuildFinalKernel BuildLoader

BuildLibrary:
	make -C $(KRNLIBRARYFOLDER) all

BuildKernel:
	make -C $(KERNELFOLDER) all

BuildArchitecture:
	make -C $(ARCHFOLDER) all

BuildDrivers:
	make -C $(DRIVERSFOLDER) all

BuildLoader:
	make -C $(LOADERFOLDER) all

BuildFinalKernel:
	$(LD) -nostdlib -T $(LINKERSCRIPT) -o $(KERNEL_ELF) $(FIRSTPRIORITY_OBJECT) $(KERNEL_OBJECTS) $(ARCH_OBJECTS) $(DRIVER_OBJECTS) -L $(ROOTBINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_FINAL)

clean:
	make -C $(KRNLIBRARYFOLDER) clean
	
	make -C $(KERNELFOLDER) clean
	make -C $(ARCHFOLDER) clean
	make -C $(DRIVERSFOLDER) clean
	make -C $(LOADERFOLDER) clean
	
	rm -rf $(KERNEL_ELF)
	rm -rf $(KERNEL_FINAL)
	rm -rf $(TARGET)

run: virtualbox

qemu:
	$(QEMU) -cdrom $(TARGET) $(QEMU_OPTION)

debug_interrupt:
	$(QEMU) -cdrom $(TARGET) $(QEMU_OPTION) -d int -M smm=off -D qemulog.txt

debug: 
	$(QEMU) -cdrom $(TARGET) $(QEMU_OPTION) -s -S -serial stdio

qemu_hd_old:
	$(QEMU) -hda $(TARGET) $(QEMU_OPTION) -boot c

debug_hd_old: 
	$(QEMU) -hda $(TARGET) $(QEMU_OPTION) -boot c -s -S -serial stdio

virtualbox:
	# qemu-img convert -O qcow2 $(TARGET) $(patsubst %.img,%.qcow2,$(TARGET))
	vboxmanage startvm "microkernel" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
