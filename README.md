# microkernel
A Basic Hobby Operating System That Might Not Actually Be Microkernel

~~(I didn't know what to name it so I just named it "microkernel".. cuz it's micro)~~
## Goal of the project
The goal of this project is to make a *very flexible* kernel(or operating system) that can be used in a variety of hardware(from Embedded Systems to Personal Computer) so that an end-user can only customize hardware specific parts of kernel code and easily develop operating system according to the detailed specification of hardware.

## Current Progress
Currently, only available architecture implemented is Intel x86_64 architecture.
Here's some current progress and future plans (decoratively displayed as To-do list)
### Kernel Loader: 
- [X] GRUB Loader
- [X] EFI Loader
- [ ] Other loaders for architectures other than x86_64
### Memory Management: 
- [X] Physical Memory Allocator
- [ ] Advanced Physical Memory Allocator
- [X] Higher half kernel  (Introduced around March 2026)
- [X] KASan(Kernel Address Sanitization) (Introduced around April 2026)
- [ ] Paging system (+ Page Frame Manager)
### Interrupt: 
- [X] Segmentation system
- [X] Interrupt system (+ Exception system)
- [ ] Advanced Interrupt management (APIC, multi-architecture support test)
### Device Drivers: 
- [X] Device Driver & Storage Driver Manager
- [X] File System Manager
- [X] Virtual File System(VFS)
- [ ] Better VFS
- [X] FAT12, FAT16, FAT32 File System
- [ ] ISO9660 File System
- [ ] Proper keyboard/mouse driver (driver for general I/O devices)
### Task Management:
- [ ] Kernel Scheduler
- [ ] Synchronization(Mutex, spinlock)
- [ ] Multicore Support, SMP(Symmetric Multiprocessing)
- [ ] System Call
- [ ] Program Execution(ELF Binary handler, etc.)
### Graphic:
- [ ] Basic shell (Integrated to kernel)
- [ ] Graphics interface NOT integrated to kernel
### Miscellaneous: 
- [ ] Documenting kernel's specific system (for end-users' accessibility to kernel's systems)
- [X] ~~Much more flexible way to compile project~~
- [ ] Much much more flexible way to compile project

## Future Plan
The ultimate goal is implementing not only x86_64 architecture, but also ARM, atmel, etc.. I am not sure whether making an operating system that can fit on any hardware is even a possible thing. I will still try as much as I can though. 
