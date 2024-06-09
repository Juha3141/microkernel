[BITS 32]

global entry
extern main

entry:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    call main

    jmp $