##### Required Arguments : 
# COMPILE_TYPE[kernel/driver]
# SEARCH_LOCATION (only for kernel option)
# INCLUDEPATHS 

include $(ROOTDIR)/configurations.mk
include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/$(ARCH_CONFIGURATION_MAKEFILE)

ASM_EXTENSION = .S

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(CURRENTFOLDER)

ifeq ($(COMPILE_TYPE),kernel)
# COMPILE_TYPE = kernel
SUBDIRECTORIES = $(subst $(COMMON_SRCFOLDER),$(MAINBINARYFOLDER),$(sort $(dir $(wildcard $(COMMON_SRCFOLDER)/*/))))
MAINTARGETS = $(subst .cpp,.obj,$(wildcard $(SEARCH_LOCATION)/*.cpp) $(wildcard $(SEARCH_LOCATION)/*/*.cpp))
ASMTARGETS = $(subst $(ASM_EXTENSION),.aobj,$(wildcard $(SEARCH_LOCATION)/*$(ASM_EXTENSION)))

else
# COMPILE_TYPE = driver
SUBDIRECTORIES = $(sort $(dir $(wildcard ./*/)))
MAINTARGETS = $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*.cpp),$(subst .cpp,.obj,$(wildcard $(dir)*.cpp))))
ASMTARGETS = $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*$(ASM_EXTENSION)),$(subst .$(ASM_EXTENSION),.aobj,$(wildcard $(dir)*$(ASM_EXTENSION)))))

endif

all: prepare $(MAINTARGETS) $(ASMTARGETS)

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
	$(KERNEL_CC) -c $< -o $(subst $(SEARCH_LOCATION),$(MAINBINARYFOLDER),$@) $(KERNEL_CCOPTIONS) $(INCLUDEPATHS)
else
	$(KERNEL_CC) -c $< -o $(MAINBINARYFOLDER)/$@ $(KERNEL_CCOPTIONS) $(INCLUDEPATHS)
endif

%.aobj: %$(ASM_EXTENSION)
	$(KERNEL_AS) $< $(KERNEL_ASOPTIONS) -o $(subst $(SEARCH_LOCATION),$(MAINBINARYFOLDER),$@)

.PHONY: clean all 