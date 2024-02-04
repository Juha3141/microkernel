SOURCESFOLDER = Sources
HEADERSFOLDER = Headers

CC = x86_64-elf-g++
ASM = nasm
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy

CCOPTIONS = -g -ffreestanding \
-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
-fpack-struct=1 -masm=intel -nostdlib \
-Werror=return-type -fno-stack-protector \
-fno-use-cxa-atexit -fno-threadsafe-statics \
-fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
LDOPTIONS = -m elf_x86_64