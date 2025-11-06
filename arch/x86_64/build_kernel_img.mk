# build_kernel : Architecture-dependent part of compiling the final kernel image from the previously compiled object files
# objects are passed by the variable TARGET_OBJECTS, and the resulting file name of the kernel image is also passed by the variable KERNEL_ELF KERNEL_IMG. 
# Location of the library and the lists of the required library options are passed to the argument as the variable LIBRARYFOLDER and LIBRARY_OPTIONS, respectively. 
# ROOTDIR is also passed to the argument

include $(ROOTDIR)/configurations.mk
include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/$(ARCH_CONFIGURATION_MAKEFILE)

ARCH_CONFIGURATION_LINKER_HEADER = $(ARCH_CONFIGURATION_FILE_LOC)/$(ARCH_CONFIGURATION_FILE)

build_kernel_img:
	# convert the architecture configuration macros to the linker script
	
	$(KERNEL_LD) $(KERNEL_LDOPTIONS) -L $(ARCH_CONFIGURATION_FILE_LOC) -T kernel/$(KERNEL_LINKERSCRIPT) $(TARGET_OBJECTS) -o $(KERNEL_ELF)
	$(KERNEL_OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_IMG)
