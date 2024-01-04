include ../../global_variables.mk

BINARYFOLDER = ../../$(ROOTBINARYFOLDER)/$(KRNLIBRARYFOLDER)
OBJECTSBINARYFOLDER = $(BINARYFOLDER)/string

CC = x86_64-linux-gnu-g++
AR = x86_64-linux-gnu-ar

CCOPTIONS = -Os -m64 -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -masm=intel -nostdlib
MAINSOURCES = $(patsubst %.cpp,%.obj,$(wildcard *.cpp))
TARGET = $(BINARYFOLDER)/libstring.a

clean:
	rm -rf $(OBJECTSBINARYFOLDER)/*.*
	rm -rf $(TARGET)

all: $(MAINSOURCES)
	$(AR) rcs $(TARGET) $(wildcard $(OBJECTSBINARYFOLDER)/*.obj)

%.obj:
	$(CC) -m64 -c $*.cpp -o $(OBJECTSBINARYFOLDER)/$@ $(CCOPTIONS)