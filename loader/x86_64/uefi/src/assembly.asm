[BITS 64]

SECTION .text

global jump_to_kernel

jump_to_kernel:
    mov rdi , rdi
    mov rax , rsi

    mov rbp , rdx
    mov rsp , rdx
    jmp rax
    
    jmp $