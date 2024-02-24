ROOTDIR = ..

include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/common_compilers.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(KERNELFOLDER)

MAINTARGETS = $(subst .cpp,.obj,$(wildcard $(COMMON_SRCFOLDER)/*.cpp) $(wildcard $(COMMON_SRCFOLDER)/*/*.cpp))
ASMTARGETS = $(subst .cpp,.aobj,$(wildcard $(COMMON_SRCFOLDER)/*.asm))
SUBDIRECTORIES = $(subst $(COMMON_SRCFOLDER),$(MAINBINARYFOLDER),$(sort $(dir $(wildcard $(COMMON_SRCFOLDER)/*/))))

all: prepare $(ASMSOURCES) $(MAINTARGETS)

prepare:
	mkdir $(MAINBINARYFOLDER)
ifneq "$(SUBDIRECTORIES)" ""
	mkdir $(SUBDIRECTORIES)
endif
	
clean:
	rm -rf $(MAINBINARYFOLDER)

%.obj: %.cpp
	$(CC) -m64 -c $< -o $(subst $(COMMON_SRCFOLDER),$(MAINBINARYFOLDER),$@) $(CCOPTIONS) -I $(ROOTDIR)/$(ARCHFOLDER)/$(ARCH)/$(COMMON_HEADERFOLDER) -I $(COMMON_HEADERFOLDER)

%.aobj: %.asm
	$(ASM) $(COMMON_SRCFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)

.PHONY: clean all 