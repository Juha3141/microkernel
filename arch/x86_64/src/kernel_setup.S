.code64

.globl jump_to_kernel_main

/*
jump_to_kernel_main : Changes the stack pointer to the given value and jump to the main kernel 
  Parameter 1 (RDI) : Loader argument
  Parameter 2 (RSI) : New stack pointer for the kernel
*/

.sect "kernel_setup_stage"

.text

jump_to_kernel_main:
    mov %rsi , %rbp
    mov %rbp , %rsp

    /* RDI : LoaderArgument, nothing to change here */
    call kernel_main

    jmp .