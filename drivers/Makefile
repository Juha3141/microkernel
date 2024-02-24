ROOTDIR = ..

include $(ROOTDIR)/global_variables.mk
include $(ROOTDIR)/common_compilers.mk

BINARYFOLDER = $(ROOTDIR)/$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(DRIVERSFOLDER)

COMPLETE_TARGETS = $(foreach dir,$(DRIVERS),$(if $(wildcard $(dir)*.cpp),$(subst .cpp,.obj,$(wildcard $(dir)*.cpp))))
COMPLETE_ASM_TARGETS = $(foreach dir,$(DRIVERS),$(if $(wildcard $(dir)*.asm),$(subst .asm,.aobj,$(wildcard $(dir)*.asm))))
DRIVERS = $(sort $(dir $(wildcard ./*/)))

all: prepare loop $(ASMSOURCES) $(MAINTARGETS)
# $(LD) -nostdlib -T $(LINKERSCRIPT) -o $(ELF) $(FIRSTPRIORITYOBJ) $(OBJECTS) -L $(BINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
# $(OBJCOPY) -O binary $(ELF) $(KERNELTARGET)

prepare:
	mkdir $(MAINBINARYFOLDER)
	mkdir $(subst .,$(MAINBINARYFOLDER),$(DRIVERS))

loop:
	make $(COMPLETE_TARGETS) $(COMPLETE_ASM_TARGETS)
	
clean:
	rm -rf $(MAINBINARYFOLDER)

%.obj: %.cpp
	$(CC) -m64 -c $< -o $(MAINBINARYFOLDER)/$@ $(CCOPTIONS) -I $(ROOTDIR)/$(KERNELFOLDER)/$(COMMON_HEADERFOLDER) -I $(ROOTDIR)/$(ARCHFOLDER)/$(ARCH)/$(COMMON_HEADERFOLDER) -I $(dir $<)

%.aobj: %.asm
	$(ASM) $(COMMON_SRCFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)

.PHONY: clean all 