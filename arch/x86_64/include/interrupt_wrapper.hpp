#ifndef _INTERRUPT_WRAPPER_HPP_
#define _INTERRUPT_WRAPPER_HPP_

#include <kernel/types.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/predeclared_interrupt_handlers.hpp>

#define INTERRUPT_START \
/* qword regs_ptr; */ /* use RSI register for regs_ptr */  \
__asm__ ("push rax"); \
__asm__ ("mov rax , [rsp+(8*1)]"); /* Save RIP*/ \
__asm__ ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rip))); \
__asm__ ("mov rax , [rsp+(8*2)]"); /* Save CS */ \
__asm__ ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , cs))); \
__asm__ ("mov rax , [rsp+(8*3)]"); /* Save RFlags */ \
__asm__ ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rflags))); \
__asm__ ("mov rax , [rsp+(8*4)]"); /* Save the original RSP */ \
__asm__ ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rsp))); \
__asm__ ("mov rax , [rsp+(8*5)]"); /* Save SS */ \
__asm__ ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , ss))); \
__asm__ ("sub rsp , %c0"::"i"(sizeof(struct Registers))); \
__asm__ ("mov [rsp+%c0] , rbx"::"i"(offsetof(struct Registers , rbx))); \
__asm__ ("mov [rsp+%c0] , rcx"::"i"(offsetof(struct Registers , rcx))); \
__asm__ ("mov [rsp+%c0] , rdx"::"i"(offsetof(struct Registers , rdx))); \
__asm__ ("mov [rsp+%c0] , rdi"::"i"(offsetof(struct Registers , rdi))); \
__asm__ ("mov [rsp+%c0] , rsi"::"i"(offsetof(struct Registers , rsi))); \
__asm__ ("mov [rsp+%c0] , r8"::"i"(offsetof(struct Registers , r8))); \
__asm__ ("mov [rsp+%c0] , r9"::"i"(offsetof(struct Registers , r9))); \
__asm__ ("mov [rsp+%c0] , r10"::"i"(offsetof(struct Registers , r10))); \
__asm__ ("mov [rsp+%c0] , r11"::"i"(offsetof(struct Registers , r11))); \
__asm__ ("mov [rsp+%c0] , r12"::"i"(offsetof(struct Registers , r12))); \
__asm__ ("mov [rsp+%c0] , r13"::"i"(offsetof(struct Registers , r13))); \
__asm__ ("mov [rsp+%c0] , r14"::"i"(offsetof(struct Registers , r14))); \
__asm__ ("mov [rsp+%c0] , r15"::"i"(offsetof(struct Registers , r15))); \
__asm__ ("mov [rsp+%c0] , rbp"::"i"(offsetof(struct Registers , rbp))); \
__asm__ ("mov rax , ds"); \
__asm__ ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , ds))); \
__asm__ ("mov rax , es"); \
__asm__ ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , es))); \
__asm__ ("mov rax , fs"); \
__asm__ ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , fs))); \
__asm__ ("mov rax , gs"); \
__asm__ ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , gs))); \
__asm__ ("mov rax , cr3"); \
__asm__ ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , cr3))); \
__asm__ ("add rsp , %c0"::"i"(sizeof(struct Registers))); \
__asm__ ("pop rax"); \
__asm__ ("mov [rsp-8+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rax))); \
/* use RSI register for regs_ptr */ \
__asm__ ("mov rsi , rsp"); \
__asm__ ("sub rsi , %c0"::"i"((sizeof(struct Registers)+8))); \
__asm__ ("sub rsp , %c0"::"i"(sizeof(struct Registers)+8)); \
PUSH_REGISTERS

#define INTERRUPT_END \
POP_REGISTERS \
__asm__ ("mov rax , [rsi+%c0]"::"i"(offsetof(struct Registers , rax))); \
__asm__ ("mov rbx , [rsi+%c0]"::"i"(offsetof(struct Registers , rbx))); \
__asm__ ("mov rcx , [rsi+%c0]"::"i"(offsetof(struct Registers , rcx))); \
__asm__ ("mov rdx , [rsi+%c0]"::"i"(offsetof(struct Registers , rdx))); \
/* Problem: because the regs_ptr is saved in the register, when the register value changes the regs_ptr also changes. */ \
__asm__ ("mov rdi , [rsi+%c0]"::"i"(offsetof(struct Registers , rdi))); \
__asm__ ("mov r8 , [rsi+%c0]"::"i"(offsetof(struct Registers , r8))); \
__asm__ ("mov r9 , [rsi+%c0]"::"i"(offsetof(struct Registers , r9))); \
__asm__ ("mov r10 , [rsi+%c0]"::"i"(offsetof(struct Registers , r10))); \
__asm__ ("mov r11 , [rsi+%c0]"::"i"(offsetof(struct Registers , r11))); \
__asm__ ("mov r12 , [rsi+%c0]"::"i"(offsetof(struct Registers , r12))); \
__asm__ ("mov r13 , [rsi+%c0]"::"i"(offsetof(struct Registers , r13))); \
__asm__ ("mov r14 , [rsi+%c0]"::"i"(offsetof(struct Registers , r14))); \
__asm__ ("mov r15 , [rsi+%c0]"::"i"(offsetof(struct Registers , r15))); \
__asm__ ("mov rbp , [rsi+%c0]"::"i"(offsetof(struct Registers , rbp))); \
__asm__ ("push rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , ss)))); \
__asm__ ("mov ss , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , ds)))); \
__asm__ ("mov ds , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , es)))); \
__asm__ ("mov es , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , fs)))); \
__asm__ ("mov fs , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , gs)))); \
__asm__ ("mov gs , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rip))));  \
__asm__ ("mov [rsp+%c0+(8*1)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RIP */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , cs)))); \
__asm__ ("mov [rsp+%c0+(8*2)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load CS */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rflags)))); \
__asm__ ("mov [rsp+%c0+(8*3)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RFlags */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rsp)))); \
__asm__ ("mov [rsp+%c0+(8*4)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RSP */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , ss)))); \
__asm__ ("mov [rsp+%c0+(8*5)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load SS */ \
__asm__ ("pop rax"); \
__asm__ ("add rsp , %c0"::"i"(sizeof(struct Registers)+8)); \
__asm__ ("iretq");

#define EXCEPTION_HANDLER_FUNCTION_DEFINITION(handler_num) \
__attribute__ ((naked)) void exception::handlers::handler##handler_num(void) {\
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num)); \
    __asm__ ("call %0"::"i"((max_t)archindep_general_interrupt_handler)); \
    __asm__ ("iretq"); \
}

#define INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_FUNCTION(handler_num) \
__attribute__ ((naked)) void interrupt::handler::general_wrapper##handler_num(void) { \
    INTERRUPT_START \
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num)); \
    __asm__ ("call %0"::"i"((max_t)archindep_general_interrupt_handler)); \
    INTERRUPT_END \
}

#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_FUNCTION(handler_num) \
__attribute__ ((naked)) void interrupt::handler::hardware_specified##handler_num(void) { \
    INTERRUPT_START \
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num)); \
    __asm__ ("call %0"::"i"((max_t)archindep_hardware_specified_interrupt_handler)); \
    INTERRUPT_END \
}

#define PUSH_REGISTERS \
__asm__ ("push rax"); \
__asm__ ("push rbx"); \
__asm__ ("push rcx"); \
__asm__ ("push rdx"); \
__asm__ ("push rdi"); \
__asm__ ("push rsi"); \
__asm__ ("push r8"); \
__asm__ ("push r9"); \
__asm__ ("push r10"); \
__asm__ ("push r11"); \
__asm__ ("push r12"); \
__asm__ ("push r13"); \
__asm__ ("push r14"); \
__asm__ ("push r15");

#define POP_REGISTERS \
__asm__ ("pop r15"); \
__asm__ ("pop r14"); \
__asm__ ("pop r13"); \
__asm__ ("pop r12"); \
__asm__ ("pop r11"); \
__asm__ ("pop r10"); \
__asm__ ("pop r9"); \
__asm__ ("pop r8"); \
__asm__ ("pop rsi"); \
__asm__ ("pop rdi"); \
__asm__ ("pop rdx"); \
__asm__ ("pop rcx"); \
__asm__ ("pop rbx"); \
__asm__ ("pop rax");


#endif