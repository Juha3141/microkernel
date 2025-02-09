# build_common.mk

##### Required arguments : 
# LOADER_CC : gcc(or g++) compiler for the loader
# LOADER_LD : the ld compiler for the loader
# LOADER_AS : the assembler "
# LOADER_CCOPTIONS : gcc(or g++) compiler options
# LOADER_ASOPTIONS : assembler options
# LOADER_LDOPTIONS : ld compiler options
# LOADER_LINKERSCRIPT : path to the linker script
# LOADER_LD_LIBRARIES : libraries options for loader
# LOADER_LD_OUT       : output file name for the ld compiler
# C_EXTENSION         : C/C++ source file's extension
# AS_EXTENSION        : assembly source file's extension

# SOURCESFOLDER	   : the path to the source folder
# INCLUDEPATHS     : includepaths for the C/C++ sources

# LINK_OBJECTS[yes/no] : select whether the compiled C/C++ objects will be linked
#                        If this options is set to "yes," compiled objects are linked using the ld options. 

##### Things to implement : 
#
# remove_target:
# 
# build_bootloader:
# 
# build_final_image:
# 
# run_os:
# 
# debug_run_os: 

include $(ROOTDIR)/configurations.mk
include $(ROOTDIR)/global_variables.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)/$(LOADERFOLDER)/$(ARCH)/$(LOADER)

clean: remove_target
	rm -rf $(BINARYFOLDER)

AUTO_CC_TARGETS = $(subst .$(C_EXTENSION),.obj,$(wildcard $(SOURCESFOLDER)/*.$(C_EXTENSION)))
AUTO_AS_TARGETS = $(subst .$(AS_EXTENSION),.aobj,$(wildcard $(SOURCESFOLDER)/*.$(AS_EXTENSION)))

ifeq ($(LINK_OBJECTS),yes)
all: prepare build_bootloader $(AUTO_CC_TARGETS) $(AUTO_AS_TARGETS) build_objects build_final_image
else
all: prepare build_bootloader $(AUTO_CC_TARGETS) $(AUTO_AS_TARGETS) build_final_image
endif

prepare:
	mkdir -p $(BINARYFOLDER)
	mkdir -p $(TARGET_DIR)

build_objects:
	$(LOADER_LD) -T $(LOADER_LINKERSCRIPT) -o $(LOADER_LD_OUT) $(LOADER_LDOPTIONS) $(wildcard $(BINARYFOLDER)/*.obj) $(wildcard $(BINARYFOLDER)/*.aobj) $(LOADER_LD_LIBRARIES)

%.obj: %.$(C_EXTENSION)
	$(LOADER_CC) -c $< -o $(subst $(SOURCESFOLDER),$(BINARYFOLDER),$@) $(LOADER_CCOPTIONS) \
		$(addprefix  -I,$(INCLUDEPATHS)) -I$(INCLUDEFOLDER) -I$(ROOTDIR)/$(ARCH_CONFIGURATION_FILE_LOC) -I$(ROOTDIR)/$(KERNELFOLDER)/$(COMMON_HEADERFOLDER)

%.aobj: %.$(AS_EXTENSION)
	$(LOADER_AS) $< -o $(subst $(SOURCESFOLDER),$(BINARYFOLDER),$@) $(LOADER_ASOPTIONS)