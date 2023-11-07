# microkernel
Very basic x86 microkernel
## Goal of the project
The goal of this project is to make a very small, portable microkernel. The kernel will contain basic memory allocation algorithms, memory protection, task management and etc. 

_The project is separated into three parts : Bootloader, Kernel and HAL(Hardware Abstraction Layer.)_
* Bootloader : grub, or any kinds of bootloader that loads kernel on a memory address. Since the bootloader loads x64 kernel, the bootloader should be (at least) 32bit program.
* Kernel     : The Microkernel
* HAL        : Functions for HAL will be provided as a form of undefined functions. The microkernel will use HAL classes to use hardware.


