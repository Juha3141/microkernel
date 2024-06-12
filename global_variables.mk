KERNELFOLDER  = kernel
ARCHFOLDER    = arch
INTEGRATEDFOLDER = integrated
FILESYSTEMFOLDER = filesystem
DRIVERSFOLDER = drivers
LOADERFOLDER  = loader
FINALIMAGEFOLDER = image

KRNLIBRARYFOLDER = library
ROOTBINARYFOLDER = bin

COMMON_HEADERFOLDER  = include
COMMON_SRCFOLDER  = src

KERNEL_ELF = Kernel.elf
KERNEL_FINAL = Kernel.krn 


PWD = $(notdir $(shell pwd))

################ customizable! ################

ARCH = x86_64
LOADER = x86_compact

COMPILER = x86_64-elf
CCOPTIONS = -g -ffreestanding -nostdlib \
-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
-fpack-struct=1 -masm=intel \
-Werror=return-type -fno-stack-protector \
-fno-use-cxa-atexit -fno-threadsafe-statics \
-fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
LDOPTIONS = -m elf_x86_64
