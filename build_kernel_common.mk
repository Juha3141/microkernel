##### Required Arguments : 
# COMPILE_TYPE[kernel/driver]
# SEARCH_LOCATION (only for kernel option)
# INCLUDEPATHS 

include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/configurations.mk
include $(ROOTDIR)/common_compilers.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(CURRENTFOLDER)

ifeq ($(COMPILE_TYPE),kernel)
# COMPILE_TYPE = kernel
SUBDIRECTORIES = $(subst $(COMMON_SRCFOLDER),$(MAINBINARYFOLDER),$(sort $(dir $(wildcard $(COMMON_SRCFOLDER)/*/))))
MAINTARGETS = $(subst .cpp,.obj,$(wildcard $(SEARCH_LOCATION)/*.cpp) $(wildcard $(SEARCH_LOCATION)/*/*.cpp))
ASMTARGETS = $(subst .asm,.aobj,$(wildcard $(SEARCH_LOCATION)/*.asm))

else
# COMPILE_TYPE = driver
SUBDIRECTORIES = $(sort $(dir $(wildcard ./*/)))
MAINTARGETS = $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*.cpp),$(subst .cpp,.obj,$(wildcard $(dir)*.cpp))))
ASMTARGETS = $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*.asm),$(subst .asm,.aobj,$(wildcard $(dir)*.asm))))

endif

all: prepare $(ASMTARGETS) $(MAINTARGETS)

prepare:
	mkdir $(MAINBINARYFOLDER)

ifneq "$(SUBDIRECTORIES)" ""
	mkdir $(subst .,$(MAINBINARYFOLDER),$(SUBDIRECTORIES))
	mkdir -p $(SUBDIRECTORIES)
endif

clean: 
	rm -rf $(MAINBINARYFOLDER)

%.obj: %.cpp
ifeq ($(COMPILE_TYPE),kernel)
	$(KERNEL_CC) -m64 -c $< -o $(subst $(SEARCH_LOCATION),$(MAINBINARYFOLDER),$@) $(KERNEL_CCOPTIONS) $(INCLUDEPATHS)
else
	$(KERNEL_CC) -m64 -c $< -o $(MAINBINARYFOLDER)/$@ $(KERNEL_CCOPTIONS) $(INCLUDEPATHS)
endif

%.aobj: %.asm
	$(KERNEL_AS) $(COMMON_SRCFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)

.PHONY: clean all 