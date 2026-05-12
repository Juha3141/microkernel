#ifndef _INTERRUPT_WRAPPER_HPP_
#define _INTERRUPT_WRAPPER_HPP_

#include <kernel/types.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/predeclared_interrupt_handlers.hpp>

// Save all the other registers(registers other than RIP,CS,RFlags,RSP,SS) to the stack
// Use RSI register as the pointer to the created Register structure
#define SAVE_REGISTERS \
__asm__ ("sub rsp , %c0"::"i"(sizeof(Registers))); \
__asm__ ("mov qword ptr[rsp+%c0] , rbx"::"i"(offsetof(Registers , rbx))); \
__asm__ ("mov qword ptr[rsp+%c0] , rcx"::"i"(offsetof(Registers , rcx))); \
__asm__ ("mov qword ptr[rsp+%c0] , rdx"::"i"(offsetof(Registers , rdx))); \
__asm__ ("mov qword ptr[rsp+%c0] , rdi"::"i"(offsetof(Registers , rdi))); \
__asm__ ("mov qword ptr[rsp+%c0] , rsi"::"i"(offsetof(Registers , rsi))); \
__asm__ ("mov qword ptr[rsp+%c0] , r8"::"i"(offsetof(Registers , r8))); \
__asm__ ("mov qword ptr[rsp+%c0] , r9"::"i"(offsetof(Registers , r9))); \
__asm__ ("mov qword ptr[rsp+%c0] , r10"::"i"(offsetof(Registers , r10))); \
__asm__ ("mov qword ptr[rsp+%c0] , r11"::"i"(offsetof(Registers , r11))); \
__asm__ ("mov qword ptr[rsp+%c0] , r12"::"i"(offsetof(Registers , r12))); \
__asm__ ("mov qword ptr[rsp+%c0] , r13"::"i"(offsetof(Registers , r13))); \
__asm__ ("mov qword ptr[rsp+%c0] , r14"::"i"(offsetof(Registers , r14))); \
__asm__ ("mov qword ptr[rsp+%c0] , r15"::"i"(offsetof(Registers , r15))); \
__asm__ ("mov qword ptr[rsp+%c0] , rbp"::"i"(offsetof(Registers , rbp))); \
__asm__ ("mov rax , ds"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , ds))); \
__asm__ ("mov rax , es"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , es))); \
__asm__ ("mov rax , fs"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , fs))); \
__asm__ ("mov rax , gs"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , gs))); \
__asm__ ("mov rax , cr3"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , cr3))); \
__asm__ ("mov rax , cr0"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , cr0))); \
__asm__ ("mov rax , cr2"); \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(offsetof(Registers , cr2))); \
/* use RSI register for regs_ptr */ \
__asm__ ("mov rsi , rsp");

#define EXCEPTION_START \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rax))); /* Save RAX */ \
__asm__ ("mov rax , qword ptr[rsp]");       /* Save CPU Error Code */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , error_code))); \
__asm__ ("mov rax , qword ptr[rsp+(8*1)]"); /* Save RIP */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rip))); \
__asm__ ("mov rax , qword ptr[rsp+(8*2)]"); /* Save CS */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , cs))); \
__asm__ ("mov rax , qword ptr[rsp+(8*3)]"); /* Save RFlags */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rflags))); \
__asm__ ("mov rax , qword ptr[rsp+(8*4)]"); /* Save the original RSP */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rsp))); \
__asm__ ("mov rax , qword ptr[rsp+(8*5)]"); /* Save SS */ \
SAVE_REGISTERS

#define INTERRUPT_START \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rax))); /* Save RAX */ \
__asm__ ("mov rax , qword ptr[rsp]");       /* Save RIP */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rip))); \
__asm__ ("mov rax , qword ptr[rsp+(8*1)]"); /* Save CS */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , cs))); \
__asm__ ("mov rax , qword ptr[rsp+(8*2)]"); /* Save RFlags */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rflags))); \
__asm__ ("mov rax , qword ptr[rsp+(8*3)]"); /* Save the original RSP */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , rsp))); \
__asm__ ("mov rax , qword ptr[rsp+(8*4)]"); /* Save SS */ \
__asm__ ("mov qword ptr[rsp+%c0] , rax"::"i"(-sizeof(Registers)+offsetof(Registers , ss))); \
SAVE_REGISTERS

#define INTERRUPT_END \
__asm__ ("mov rbx , [rsi+%c0]"::"i"(offsetof(Registers , rbx))); \
__asm__ ("mov rcx , [rsi+%c0]"::"i"(offsetof(Registers , rcx))); \
__asm__ ("mov rdx , [rsi+%c0]"::"i"(offsetof(Registers , rdx))); \
__asm__ ("mov rdi , [rsi+%c0]"::"i"(offsetof(Registers , rdi))); \
__asm__ ("mov r8 , [rsi+%c0]"::"i"(offsetof(Registers , r8))); \
__asm__ ("mov r9 , [rsi+%c0]"::"i"(offsetof(Registers , r9))); \
__asm__ ("mov r10 , [rsi+%c0]"::"i"(offsetof(Registers , r10))); \
__asm__ ("mov r11 , [rsi+%c0]"::"i"(offsetof(Registers , r11))); \
__asm__ ("mov r12 , [rsi+%c0]"::"i"(offsetof(Registers , r12))); \
__asm__ ("mov r13 , [rsi+%c0]"::"i"(offsetof(Registers , r13))); \
__asm__ ("mov r14 , [rsi+%c0]"::"i"(offsetof(Registers , r14))); \
__asm__ ("mov r15 , [rsi+%c0]"::"i"(offsetof(Registers , r15))); \
__asm__ ("mov rbp , [rsi+%c0]"::"i"(offsetof(Registers , rbp))); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , ss)))); \
__asm__ ("mov ss , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , ds)))); \
__asm__ ("mov ds , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , es)))); \
__asm__ ("mov es , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , fs)))); \
__asm__ ("mov fs , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , gs)))); \
__asm__ ("mov gs , rax"); \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , rip))));  \
__asm__ ("mov [rsp+%c0] , rax"::"i"(sizeof(Registers)));     /* Load RIP */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , cs)))); \
__asm__ ("mov [rsp+%c0+(8*1)] , rax"::"i"(sizeof(Registers)));     /* Load CS */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , rflags)))); \
__asm__ ("mov [rsp+%c0+(8*2)] , rax"::"i"(sizeof(Registers)));     /* Load RFlags */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , rsp)))); \
__asm__ ("mov [rsp+%c0+(8*3)] , rax"::"i"(sizeof(Registers)));     /* Load RSP */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"((offsetof(Registers , ss)))); \
__asm__ ("mov [rsp+%c0+(8*4)] , rax"::"i"(sizeof(Registers)));     /* Load SS */ \
__asm__ ("mov rax , [rsi+%c0]"::"i"(offsetof(Registers , rax)));  /* Load RAX*/ \
__asm__ ("mov rsi , [rsi+%c0]"::"i"(offsetof(Registers , rsi)));  /* Load RSI*/ \
__asm__ ("add rsp , %c0"::"i"(sizeof(Registers))); \
__asm__ ("iretq");

#define EXCEPTION_HANDLER_FUNCTION_DEFINITION(handler_num)                      \
__attribute__ ((naked)) void exception::handlers::handler##handler_num(void) {  \
    EXCEPTION_START                                                             \
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num));                          \
    __asm__ ("call archindep_general_exception_handler");                       \
}

#define INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_FUNCTION(handler_num)                     \
__attribute__ ((naked)) void interrupt::handler::general_wrapper##handler_num(void) {   \
    INTERRUPT_START                                                                     \
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num));                                  \
    __asm__ ("call archindep_general_interrupt_handler");                               \
    /* Return(RAX) : Register list that will be restored to current context */          \
    __asm__ ("mov rsi , rax");                                                          \
    INTERRUPT_END                                                                       \
}

#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_FUNCTION(handler_num)                  \
__attribute__ ((naked)) void interrupt::handler::hardware_specified##handler_num(void) {    \
    INTERRUPT_START                                                                         \
    __asm__ ("mov rdi , %0"::"i"((max_t)handler_num));                                      \
    __asm__ ("call archindep_hardware_specified_interrupt_handler");                        \
    /* Return(RAX) : Register list that will be restored to current context */          \
    __asm__ ("mov rsi , rax");                                                          \
    INTERRUPT_END                                                                           \
}


#endif