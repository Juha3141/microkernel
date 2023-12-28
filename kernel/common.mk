SOURCESFOLDER = Sources
HEADERSFOLDER = Headers

CC = x86_64-elf-g++
ASM = nasm
OBJCOPY = x86_64-elf-objcopy

# -fPIC : Position Independent Code
CCOPTIONS = -g -ffreestanding \
-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
-fpack-struct=1  -masm=intel -nostdlib \
-Werror=return-type -fno-stack-protector \
-fno-use-cxa-atexit -fno-threadsafe-statics \
-fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
LDOPTIONS = -m elf_x86_64