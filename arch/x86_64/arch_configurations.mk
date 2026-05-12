AS = as
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

# To-do : integrate conf file with makefile configurations
KASAN_OFFSET  = 0x2fa000000000
KASAN_OPTIONS := -fsanitize=kernel-address
KASAN_OPTIONS += -mllvm -asan-stack=1
KASAN_OPTIONS += -mllvm -asan-globals=1
KASAN_OPTIONS += -mllvm -asan-mapping-offset=$(KASAN_OFFSET)


COMMON_CCOPTIONS := --target=x86_64-pc-linux-gnu
COMMON_CCOPTIONS += -march=x86-64 -Werror=vla-cxx-extension
COMMON_CCOPTIONS += -ffreestanding -fno-builtin -nostdlib -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -nostdinc++ 
COMMON_CCOPTIONS += -fno-common -fno-pie -fno-PIE -fno-pic -fno-PIC
COMMON_CCOPTIONS += -fpack-struct=1 -masm=intel -std=c++20 -fno-stack-protector
COMMON_CCOPTIONS += -Werror=return-type -Wno-incompatible-library-redeclaration 
COMMON_CCOPTIONS += -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-threadsafe-statics -g -mcmodel=large

KERNEL_SETUP_CCOPTIONS = -fno-sanitize=kernel-address
KERNEL_CCOPTIONS       = $(KASAN_OPTIONS)

KERNEL_LDOPTIONS = -m elf_x86_64 -nostdlib -z noexecstack
KERNEL_ASOPTIONS = 