include ../global_variables.mk
include common.mk

BINARYFOLDER = ../$(ROOTBINARYFOLDER)
MAINBINARYFOLDER = $(BINARYFOLDER)/$(KERNELFOLDER)

MAINSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.cpp))
ASMSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.asm))

LINKERSCRIPT = Linker.ld

ELF = Kernel.elf

LIBRARIES = $(patsubst %.a,%,$(subst lib,-l,$(notdir $(wildcard $(BINARYFOLDER)/$(KRNLIBRARYFOLDER)/*.a))))
FIRSTPRIORITYOBJ = $(MAINBINARYFOLDER)/main.obj
OBJECTS = $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(MAINBINARYFOLDER)/*.obj)) $(wildcard $(MAINBINARYFOLDER)/$(HALFOLDER)/*.obj)

all: $(ASMSOURCES) $(MAINSOURCES)
	echo $(wildcard $(MAINBINARYFOLDER)/*.obj)
	
	x86_64-elf-ld -nostdlib -T $(LINKERSCRIPT) -o $(ELF) $(FIRSTPRIORITYOBJ) $(OBJECTS) -L $(BINARYFOLDER)/$(KRNLIBRARYFOLDER) $(LIBRARIES)
	x86_64-elf-objcopy -O binary $(ELF) $(KERNELTARGET)

clean:
	rm -rf $(MAINBINARYFOLDER)/*.*

%.cpp:
	$(CC) -m64 -c $(SOURCESFOLDER)/$@ -o $(MAINBINARYFOLDER)/$(subst .cpp,.obj,$@) $(CCOPTIONS) -I $(HALFOLDER)/Headers/ -I $(HEADERSFOLDER)/

%.asm:
	$(ASM) $(SOURCESFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)