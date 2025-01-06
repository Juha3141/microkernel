[BITS 32]

SECTION .text

global Entry
extern Main

Entry:
    cli

    mov ebp , KernelStack
    mov esp , KernelStack
    push ebx ; Multiboot information pointer

    call Main
    
    jmp $

SECTION .bss

    resb 4096
KernelStack: