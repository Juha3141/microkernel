[BITS 32]

SECTION .text

magic_value equ 0x1BADB002
flags_value equ (0 << 2)|(1 << 0) ; flag 0 : align 4kb, flag 2 : GUI

; Multiboot Header
	align 0x04 
	magic:         dd magic_value
	flags:         dd flags_value
	checksum:      dd -(magic_value+flags_value)

    header_addr:   dd 0x00
    load_addr:     dd 0x00
    load_end_addr: dd 0x00
    bss_end_addr:  dd 0x00
    entry_addr:    dd 0x00

    mode_type:     dd 0x00
    width:         dd 1024
    height:        dd 768
    depth:         dd 32

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