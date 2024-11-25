ARCH = x86_64
IMAGE = grub_iso

KERNEL_COMPILER = x86_64-elf
KERNEL_CCOPTIONS = -g -ffreestanding -nostdlib \
-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
-fpack-struct=1 -masm=intel \
-Werror=return-type -fno-stack-protector \
-fno-use-cxa-atexit -fno-threadsafe-statics \
-fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
KERNEL_LDOPTIONS = -m elf_x86_64
KERNEL_ASOPTIONS = -f elf64

OUTPUT_FILENAME = OS

AS = nasm
CC = g++
LD = ld
OBJDUMP = objdump
OBJCOPY = objcopy