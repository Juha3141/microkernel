include ../global_variables.mk
include common.mk

BINARYFOLDER = ../$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(KERNELFOLDER)

MAINTARGETS = $(subst .cpp,.obj,$(wildcard $(SOURCESFOLDER)/*.cpp) $(wildcard $(SOURCESFOLDER)/*/*.cpp))
ASMTARGETS = $(subst .cpp,.aobj,$(wildcard $(SOURCESFOLDER)/*.asm))

LINKERSCRIPT = Linker.ld

ELF = Kernel.elf

LIBRARIES = $(patsubst %.a,%,$(subst lib,-l,$(notdir $(wildcard $(BINARYFOLDER)/$(KRNLIBRARYFOLDER)/*.a))))
FIRSTPRIORITYOBJ = $(MAINBINARYFOLDER)/main.obj
OBJECTS = $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(MAINBINARYFOLDER)/*.obj)) $(wildcard $(MAINBINARYFOLDER)/$(HARDWAREFOLDER)/*.obj)  $(wildcard $(MAINBINARYFOLDER)/integrated/*.obj) \
$(wildcard $(MAINBINARYFOLDER)/drivers/*.obj)

all: $(ASMSOURCES) $(MAINTARGETS)
	echo $(MAINSOURCES)
	$(LD) -nostdlib -T $(LINKERSCRIPT) -o $(ELF) $(FIRSTPRIORITYOBJ) $(OBJECTS) -L $(BINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
	$(OBJCOPY) -O binary $(ELF) $(KERNELTARGET)

clean:
	rm -rf $(MAINBINARYFOLDER)/*.*
	rm -rf $(MAINBINARYFOLDER)/*/*.*

%.obj: %.cpp
	$(CC) -m64 -c $< -o $(subst $(SOURCESFOLDER),$(KERNELFOLDER),$(BINARYFOLDER)/$@) $(CCOPTIONS) -I $(HARDWAREFOLDER)/Headers/ -I $(HEADERSFOLDER)/

%.aobj: %.asm
	$(ASM) $(SOURCESFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)

.PHONY: clean all 