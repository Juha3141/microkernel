include ../../global_variables.mk
include ../common.mk

BINARYFOLDER = ../../$(ROOTBINARYFOLDER)/$(KERNELFOLDER)/$(HARDWAREFOLDER)
KERNELINCLUDE = ..

MAINSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.cpp))
ASMSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.asm))

all: $(ASMSOURCES) $(MAINSOURCES)
	
clean:
	rm -rf $(BINARYFOLDER)/*.*
	
%.cpp:
	$(CC) -m64 -c $(SOURCESFOLDER)/$@ -o $(BINARYFOLDER)/$(subst .cpp,.obj,$@) $(CCOPTIONS) -I $(HEADERSFOLDER)/ -I $(KERNELINCLUDE)/Headers/

%.asm:
	$(ASM) $(SOURCESFOLDER)/$@ -f elf64 -o $(BINARYFOLDER)/$(subst .asm,.obj,$@)