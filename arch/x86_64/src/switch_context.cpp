#include <arch_inline_asm.hpp>
#include <registers.hpp>
#include <arch/switch_context.hpp>
#include <kernel/essentials.hpp>

__attribute__ ((naked)) void switch_context(struct Registers *current_context , struct Registers *next_context) {
    // prev_context : rdi
    // next_context : rsi
    /* Save */
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , rax)));
    __asm__ ("mov [rdi+%0] , rbx"::"i"(offsetof(struct Registers , rbx)));
    __asm__ ("mov [rdi+%0] , rcx"::"i"(offsetof(struct Registers , rcx)));
    __asm__ ("mov [rdi+%0] , rdx"::"i"(offsetof(struct Registers , rdx)));

    __asm__ ("mov [rdi+%0] , rdi"::"i"(offsetof(struct Registers , rdi)));
    __asm__ ("mov [rdi+%0] , rsi"::"i"(offsetof(struct Registers , rsi)));

    __asm__ ("mov [rdi+%0] , r8"::"i"(offsetof(struct Registers , r8)));
    __asm__ ("mov [rdi+%0] , r9"::"i"(offsetof(struct Registers , r9)));
    __asm__ ("mov [rdi+%0] , r10"::"i"(offsetof(struct Registers , r10)));
    __asm__ ("mov [rdi+%0] , r11"::"i"(offsetof(struct Registers , r11)));
    __asm__ ("mov [rdi+%0] , r12"::"i"(offsetof(struct Registers , r12)));
    __asm__ ("mov [rdi+%0] , r13"::"i"(offsetof(struct Registers , r13)));
    __asm__ ("mov [rdi+%0] , r14"::"i"(offsetof(struct Registers , r14)));
    __asm__ ("mov [rdi+%0] , r15"::"i"(offsetof(struct Registers , r15)));

    __asm__ ("mov [rdi+%0] , rbp"::"i"(offsetof(struct Registers , rbp)));
    __asm__ ("lea rax , [rsp+8]"); /* value of rsp before pushing the return address to the stack */
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , rsp)));
    
    __asm__ ("mov rax , cs");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , cs)));
    __asm__ ("mov rax , ss");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , ss)));
    __asm__ ("mov rax , ds");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , ds)));
    __asm__ ("mov rax , es");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , es)));
    __asm__ ("mov rax , fs");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , fs)));
    __asm__ ("mov rax , gs");
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , gs)));

    __asm__ ("pushfq");          // rflags
    __asm__ ("pop [rdi+%0]"::"i"(offsetof(struct Registers , rflags)));
    __asm__ ("pop rax");         // rip
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , rip)));
    __asm__ ("mov rax , cr3");   // cr3
    __asm__ ("mov [rdi+%0] , rax"::"i"(offsetof(struct Registers , cr3)));

    /* Load */
    __asm__ ("mov rbx , [rsi+%0]"::"i"(offsetof(struct Registers , rbx)));
    __asm__ ("mov rcx , [rsi+%0]"::"i"(offsetof(struct Registers , rcx)));
    __asm__ ("mov rdx , [rsi+%0]"::"i"(offsetof(struct Registers , rbx)));

    __asm__ ("mov rdi , [rsi+%0]"::"i"(offsetof(struct Registers , rdi)));

    __asm__ ("mov r8 , [rsi+%0]"::"i"(offsetof(struct Registers , r8)));
    __asm__ ("mov r9 , [rsi+%0]"::"i"(offsetof(struct Registers , r9)));
    __asm__ ("mov r10 , [rsi+%0]"::"i"(offsetof(struct Registers , r10)));
    __asm__ ("mov r11 , [rsi+%0]"::"i"(offsetof(struct Registers , r11)));
    __asm__ ("mov r12 , [rsi+%0]"::"i"(offsetof(struct Registers , r12)));
    __asm__ ("mov r13 , [rsi+%0]"::"i"(offsetof(struct Registers , r13)));
    __asm__ ("mov r14 , [rsi+%0]"::"i"(offsetof(struct Registers , r14)));
    __asm__ ("mov r15 , [rsi+%0]"::"i"(offsetof(struct Registers , r15)));

    __asm__ ("push [rsi+%0]"::"i"(offsetof(struct Registers , ss)));     // ss
    __asm__ ("push [rsi+%0]"::"i"(offsetof(struct Registers , rsp)));    // rsp
    __asm__ ("push [rsi+%0]"::"i"(offsetof(struct Registers , rflags))); // rflags
    __asm__ ("push [rsi+%0]"::"i"(offsetof(struct Registers , cs)));     // cs
    __asm__ ("push [rsi+%0]"::"i"(offsetof(struct Registers , rip)));    // rip

    __asm__ ("mov rbp , [rsi+%0]"::"i"(offsetof(struct Registers , rbp)));

    __asm__ ("mov rax , [rsi+%0]"::"i"(offsetof(struct Registers , ds)));
    __asm__ ("mov ds , rax");
    __asm__ ("mov rax , [rsi+%0]"::"i"(offsetof(struct Registers , es)));
    __asm__ ("mov es , rax");
    __asm__ ("mov rax , [rsi+%0]"::"i"(offsetof(struct Registers , fs)));
    __asm__ ("mov fs , rax");
    __asm__ ("mov rax , [rsi+%0]"::"i"(offsetof(struct Registers , gs)));
    __asm__ ("mov gs , rax");

    __asm__ ("mov rax , [rsi+%0]"::"i"(offsetof(struct Registers , rax)));
    __asm__ ("mov rsi , [rsi+%0]"::"i"(offsetof(struct Registers , rsi)));
    __asm__ ("iretq");
}