ROOTDIR = ..
CURRENTFOLDER = filesystem

include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/common_compilers.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(CURRENTFOLDER)

COMPLETE_TARGETS = $(subst .cpp,.obj,$(wildcard *.cpp)) $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*.cpp),$(subst .cpp,.obj,$(wildcard $(dir)*.cpp))))
COMPLETE_ASM_TARGETS = $(foreach dir,$(SUBDIRECTORIES),$(if $(wildcard $(dir)*.asm),$(subst .asm,.aobj,$(wildcard $(dir)*.asm))))
SUBDIRECTORIES = $(sort $(dir $(wildcard ./*/)))

all: prepare loop $(ASMSOURCES) $(MAINTARGETS)
# $(LD) -nostdlib -T $(LINKERSCRIPT) -o $(ELF) $(FIRSTPRIORITYOBJ) $(OBJECTS) -L $(BINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
# $(OBJCOPY) -O binary $(ELF) $(KERNELTARGET)

prepare:
	mkdir $(MAINBINARYFOLDER)
	mkdir $(subst .,$(MAINBINARYFOLDER),$(SUBDIRECTORIES))

loop:
	make $(COMPLETE_TARGETS) $(COMPLETE_ASM_TARGETS)
	
clean:
	rm -rf $(MAINBINARYFOLDER)

%.obj: %.cpp
	$(CC) -m64 -c $< -o $(MAINBINARYFOLDER)/$@ $(CCOPTIONS) -I $(ROOTDIR)/$(KERNELFOLDER)/$(COMMON_HEADERFOLDER) -I $(ROOTDIR)/$(ARCHFOLDER)/$(ARCH)/$(COMMON_HEADERFOLDER) -I $(dir $<)

%.aobj: %.asm
	$(ASM) $(COMMON_SRCFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)

.PHONY: clean all 
