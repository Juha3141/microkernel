AS = nasm
CC = g++
LD = ld
OBJDUMP = objdump
OBJCOPY = objcopy

KERNEL_COMPILER = x86_64-elf

#--target=x86_64-elf -m64 -g -ffreestanding -nostdlib -std=c++20 \
-mcmodel=large -mno-mmx -mno-sse -mno-sse2 \
-fpack-struct=1 -masm=intel \
-Werror=return-type -fno-stack-protector \
-fno-use-cxa-atexit -fno-threadsafe-statics \
-fno-rtti -fno-exceptions -Wno-write-strings

KERNEL_CCOPTIONS = --target=x86_64-pc-none-elf -march=x86-64 -ffreestanding -fno-builtin -nostdlib -nostdinc++ -fpack-struct=1 -masm=intel -std=c++20 -Werror=return-type -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-stack-protector -fno-threadsafe-statics -O0 -fPIC -fPIE -g 
KERNEL_LDOPTIONS = -nostartfiles -nodefaultlibs --target=x86_64-elf -nostdlib -ffreestanding -mcmodel=large
KERNEL_ASOPTIONS = -f elf64