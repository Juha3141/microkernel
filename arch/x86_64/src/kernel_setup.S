.code64

.globl jump_to_kernel_main

/*
jump_to_kernel_main : Changes the stack pointer to the given value and jump to the main kernel 
  Parameter 1 (RDI) : Loader argument
  Parameter 2 (RSI) : New stack pointer for the kernel
*/

.sect "kernel_setup_stage"

.text

/* RDI : loader_argument
 * RSI : kernel image virtual address
 * RDX : kernel stack virtual address
 * RCX : kernel stack size
 */
jump_to_kernel_main:
  mov %rdx , %rbp  /* rbp = stack vma + stack size - 8 */
  add %rcx , %rbp
  sub $8 , %rbp
  mov %rbp , %rsp

  /* The sequence of necessary argument for kernel_main is the same as what is given to jump_to_kernel_main */
  call kernel_main

  jmp .