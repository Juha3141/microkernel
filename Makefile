include global_variables.mk
include configurations.mk

BASH = bash
QEMU = qemu-system-x86_64

KERNEL_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*.obj $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*.aobj) \
$(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*/*.obj $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*/*.aobj)
ARCH_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*.obj) $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*.aobj) \
$(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*/*.obj)
INTEGRATED_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(INTEGRATEDFOLDER)/*/*.obj) $(wildcard $(ROOTBINARYFOLDER)/$(INTEGRATEDFOLDER)/*/*.aobj)
FILESYSTEM_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(FILESYSTEMFOLDER)/*/*.obj) $(wildcard $(ROOTBINARYFOLDER)/$(FILESYSTEMFOLDER)/*/*.aobj)
KERNEL_TARGET_OBJECTS = $(FIRSTPRIORITY_OBJECT) $(KERNEL_OBJECTS) $(ARCH_OBJECTS) $(INTEGRATED_OBJECTS) $(FILESYSTEM_OBJECTS)

BUILD_TARGET_FOLDERS = $(KERNELFOLDER) $(ARCHFOLDER) $(INTEGRATEDFOLDER) $(FILESYSTEMFOLDER) 

ROOTDIR = $(PWD)
MAKE_ARGUMENTS = ROOTDIR=$(ROOTDIR)

all: build_kernel_sources build_kernel_binary build_loader

clean: clean_loader
	rm -rf $(KERNEL_ELF)
	rm -rf $(KERNEL_IMG)
	@echo "$(shell tput bold)================  Cleaning ================$(shell tput sgr0)"
	for build_dir in $(BUILD_TARGET_FOLDERS); do\
		echo Cleaning the "$$build_dir" folder...; \
		make -C $$build_dir clean $(MAKE_ARGUMENTS) CURRENTFOLDER=$$build_dir || exit; \
	done

clean_loader:
	make -C $(LOADERFOLDER)/$(ARCH)/$(LOADER) clean $(MAKE_ARGUMENTS) TARGET_DIR=$(ROOTDIR)/$(FINALIMAGEFOLDER)/$(ARCH)/$(LOADER) 

build_kernel_sources: 
	@echo "$(shell tput bold)================  Builing the full kernel sources ================$(shell tput sgr0)"
	for build_dir in $(BUILD_TARGET_FOLDERS); do\
		echo Building the "$$build_dir" folder...; \
		make -C $$build_dir all $(MAKE_ARGUMENTS) CURRENTFOLDER=$$build_dir || exit; \
	done

build_kernel_binary:
	@echo "$(shell tput bold)================  Builing the kernel binary ================$(shell tput sgr0)"
	make -f $(ARCHFOLDER)/$(ARCH)/$(BUILD_KERNEL_IMG_MAKEFILE) build_kernel_img ROOTDIR=$(PWD) TARGET_OBJECTS="$(KERNEL_TARGET_OBJECTS)"\
		LIBRARYFOLDER="." KERNEL_ELF=$(ROOTBINARYFOLDER)/$(KERNEL_ELF) KERNEL_IMG=$(KERNEL_IMG_LOCATION)/$(KERNEL_IMG)

build_loader:
	@echo "$(shell tput bold)================  Builing the final kernel image ================$(shell tput sgr0)"
	make -C $(LOADERFOLDER)/$(ARCH)/$(LOADER) all $(MAKE_ARGUMENTS) TARGET_DIR=$(ROOTDIR)/$(FINALIMAGEFOLDER)/$(ARCH)/$(LOADER) 

run: 
	make -C $(LOADERFOLDER)/$(ARCH)/$(LOADER) run_os $(MAKE_ARGUMENTS) TARGET_DIR=$(ROOTDIR)/$(FINALIMAGEFOLDER)/$(ARCH)/$(LOADER) 

debugrun:
	make -C $(LOADERFOLDER)/$(ARCH)/$(LOADER) debug_run_os $(MAKE_ARGUMENTS) TARGET_DIR=$(ROOTDIR)/$(FINALIMAGEFOLDER)/$(ARCH)/$(LOADER) 