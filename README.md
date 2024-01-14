# microkernel
Very basic x86 Operating System (microkernel)
## Goal of the project
The goal of this project is to make a *very flexible* kernel(or operating system) that can be used in a variety of hardware(from Embedded Systems to Personal Computer) so that an end-user can only customize hardware specific parts of kernel code and easily develop operating system according to the detailed specification of hardware.
To achieve this goal, the OS is separated into three parts: Bootloader, Kernel, and Hardware(currently on x86_64 architecture). 
- **Bootloader** part consists of **any type** of software that can load the kernel to a designated memory address and give basic information to the kernel (via function argument.)
- **Kernel** part consists of the management of abstract systems and resources, such as Interrupt, Paging, Segmentation, Memory, Device, etc.. The kernel should *not* directly communicate with hardware, but should be commuicating though HAL(Hardware Abstraction Layer) 
- **Hardware** part consists of (nearly) every hardware-specific system(functions) in kernel system - that is, HAL - and device driver.

## Current Progress
Currently, only available architecture implemented is Intel x86_64 architecture. Not much is actually happening for now, as even the basic device driver manager is not implemented yet.
Here's some current progress and future plans (decoratively displayed as To-do list)
- [ ] Much more flexible way to compile project
- [X] GRUB Loader
- [ ] EFI Loader
- [ ] Interface between Kernel and Hardware
- [X] Physical Memory Allocator
- [ ] Paging system (+ Page Frame Manager)
- [X] Segmentation system
- [X] Interrupt system (+ Exception system)
- [ ] Kernel Scheduler
- [ ] Device Driver & Storage Driver Manager
- [ ] File System Manager
- [ ] File System Driver(FAT, NTFS, ext..)
- [ ] Networking
(Lots of more stuff will be added in distant future)

### Future Plan
The ultimate goal is implementing not only x86_64 architecture, but also ARM, atmel, etc.. Although I am not sure whether making an operating system that can fit on any hardware is even a possible thing, I will still try developing it as much as I can. 
