include global_variables.mk
include common_compilers.mk

BASH = bash
QEMU = qemu-system-x86_64

FIRSTPRIORITY_OBJECT = $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/main.obj
KERNEL_OBJECTS = $(filter-out $(FIRSTPRIORITY_OBJECT),$(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*.obj)) $(wildcard $(ROOTBINARYFOLDER)/$(KERNELFOLDER)/*/*.obj)
ARCH_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*.obj) $(wildcard $(ROOTBINARYFOLDER)/$(ARCHFOLDER)/*/*.obj)
INTEGRATED_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(INTEGRATEDFOLDER)/*/*.obj)
FILESYSTEM_OBJECTS = $(wildcard $(ROOTBINARYFOLDER)/$(FILESYSTEMFOLDER)/*/*.obj)

LIBRARIES = $(patsubst %.a,%,$(subst lib,-l,$(notdir $(wildcard $(ROOTBINARYFOLDER)/$(KRNLIBRARYFOLDER)/*.a))))

LINKERSCRIPT = $(ARCHFOLDER)/$(ARCH)/kernel_linker.ld

all: BuildFullKernel BuildLoader

BuildLibrary:
	make -C $(KRNLIBRARYFOLDER) all

BuildKernel:
	make -C $(KERNELFOLDER) all

BuildArchitecture:
	make -C $(ARCHFOLDER) all

BuildIntegrated:
	make -C $(INTEGRATEDFOLDER) all

BuildFileSystems:
	make -C $(FILESYSTEMFOLDER) all

BuildDrivers:
	make -C $(DRIVERSFOLDER) all

BuildLoader:
	make -C $(LOADERFOLDER) all

BuildKernelBinary:
	$(LD) -nostdlib -T $(LINKERSCRIPT) -o $(KERNEL_ELF) $(FIRSTPRIORITY_OBJECT) $(KERNEL_OBJECTS) $(ARCH_OBJECTS) $(INTEGRATED_OBJECTS) $(FILESYSTEM_OBJECTS) -L $(ROOTBINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_FINAL)

BuildFullKernel: BuildLibrary BuildKernel BuildArchitecture BuildIntegrated BuildFileSystems BuildDrivers BuildKernelBinary

clean: CleanKernelLibrary CleanArch CleanFileSystem CleanIntegrated CleanKernel CleanDrivers CleanLoader

CleanKernelLibrary:
	make -C $(KRNLIBRARYFOLDER) clean

CleanKernel:
	make -C $(KERNELFOLDER) clean
	
	rm -rf $(KERNEL_ELF)
	rm -rf $(KERNEL_FINAL)

CleanFileSystem:
	make -C $(FILESYSTEMFOLDER) clean

CleanArch:
	make -C $(ARCHFOLDER) clean

CleanIntegrated:
	make -C $(INTEGRATEDFOLDER) clean

CleanDrivers:
	make -C $(DRIVERSFOLDER) clean

CleanLoader:
	make -C $(LOADERFOLDER) clean

CleanFullKernel: CleanKernelLibrary CleanKernel CleanFileSystem CleanArch CleanIntegrated CleanDrivers

run: 
	make -C $(LOADERFOLDER)/$(LOADER) run

debugrun:
	make -C $(LOADERFOLDER)/$(LOADER) debugrun
