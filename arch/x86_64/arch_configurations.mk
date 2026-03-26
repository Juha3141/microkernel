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
KASAN_OFFSET  = 0xffaa000000000
KASAN_OPTIONS = # -fsanitize=kernel-address \
	            -mllvm -asan-stack=1 \
				-mllvm -asan-globals=1 \
	            -mllvm -asan-instrumentation-with-call-threshold=0 \
				-mllvm -asan-mapping-offset=$(KASAN_OFFSET)
COMMON_CCOPTIONS = --target=x86_64-pc-linux-gnu -march=x86-64 -ffreestanding -fno-builtin -nostdlib -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -nostdinc++ -fpack-struct=1 -masm=intel -std=c++20 -Werror=return-type -Wno-incompatible-library-redeclaration -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-threadsafe-statics -O0 -g -mcmodel=large

KERNEL_SETUP_CCOPTIONS =  -fno-stack-protector -fno-pic -fno-pie
KERNEL_CCOPTIONS       =  -fPIC $(KASAN_OPTIONS)

KERNEL_LDOPTIONS = -nostartfiles -nodefaultlibs --target=x86_64-elf -nostdlib -ffreestanding
KERNEL_ASOPTIONS = 