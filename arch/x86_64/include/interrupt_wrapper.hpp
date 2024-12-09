#ifndef _INTERRUPT_WRAPPER_HPP_
#define _INTERRUPT_WRAPPER_HPP_

#include <kernel/types.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/predeclared_interrupt_handlers.hpp>

#define INTERRUPT_START \
/* qword regs_ptr; */ /* use RSI register for regs_ptr */  \
IA ("push rax"); \
IA ("mov rax , [rsp+(8*1)]"); /* Save RIP*/ \
IA ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rip))); \
IA ("mov rax , [rsp+(8*2)]"); /* Save CS */ \
IA ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , cs))); \
IA ("mov rax , [rsp+(8*3)]"); /* Save RFlags */ \
IA ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rflags))); \
IA ("mov rax , [rsp+(8*4)]"); /* Save the original RSP */ \
IA ("mov [rsp+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rsp))); \
IA ("sub rsp , %c0"::"i"(sizeof(struct Registers))); \
IA ("mov [rsp+%c0] , rbx"::"i"(offsetof(struct Registers , rbx))); \
IA ("mov [rsp+%c0] , rcx"::"i"(offsetof(struct Registers , rcx))); \
IA ("mov [rsp+%c0] , rdx"::"i"(offsetof(struct Registers , rdx))); \
IA ("mov [rsp+%c0] , rdi"::"i"(offsetof(struct Registers , rdi))); \
IA ("mov [rsp+%c0] , rsi"::"i"(offsetof(struct Registers , rsi))); \
IA ("mov [rsp+%c0] , r8"::"i"(offsetof(struct Registers , r8))); \
IA ("mov [rsp+%c0] , r9"::"i"(offsetof(struct Registers , r9))); \
IA ("mov [rsp+%c0] , r10"::"i"(offsetof(struct Registers , r10))); \
IA ("mov [rsp+%c0] , r11"::"i"(offsetof(struct Registers , r11))); \
IA ("mov [rsp+%c0] , r12"::"i"(offsetof(struct Registers , r12))); \
IA ("mov [rsp+%c0] , r13"::"i"(offsetof(struct Registers , r13))); \
IA ("mov [rsp+%c0] , r14"::"i"(offsetof(struct Registers , r14))); \
IA ("mov [rsp+%c0] , r15"::"i"(offsetof(struct Registers , r15))); \
IA ("mov [rsp+%c0] , rbp"::"i"(offsetof(struct Registers , rbp))); \
IA ("mov rax , ss"); \
IA ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , ss))); \
IA ("mov rax , ds"); \
IA ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , ds))); \
IA ("mov rax , es"); \
IA ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , es))); \
IA ("mov rax , fs"); \
IA ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , fs))); \
IA ("mov rax , gs"); \
IA ("mov [rsp+%c0] , rax"::"i"(offsetof(struct Registers , gs))); \
IA ("add rsp , %c0"::"i"(sizeof(struct Registers))); \
IA ("pop rax"); \
IA ("mov [rsp-8+%c0] , rax"::"i"(-sizeof(struct Registers)+offsetof(struct Registers , rax))); \
/* use RSI register for regs_ptr */ \
IA ("mov rsi , rsp"); \
IA ("sub rsi , %c0"::"i"((sizeof(struct Registers)+8))); \
IA ("sub rsp , %c0"::"i"(sizeof(struct Registers)+8)); \
PUSH_REGISTERS

#define INTERRUPT_END \
POP_REGISTERS \
IA ("mov rax , [rsi+%c0]"::"i"(offsetof(struct Registers , rax))); \
IA ("mov rbx , [rsi+%c0]"::"i"(offsetof(struct Registers , rbx))); \
IA ("mov rcx , [rsi+%c0]"::"i"(offsetof(struct Registers , rcx))); \
IA ("mov rdx , [rsi+%c0]"::"i"(offsetof(struct Registers , rdx))); \
/* Problem: because the regs_ptr is saved in the register, when the register value changes the regs_ptr also changes. */ \
IA ("mov rdi , [rsi+%c0]"::"i"(offsetof(struct Registers , rdi))); \
IA ("mov r8 , [rsi+%c0]"::"i"(offsetof(struct Registers , r8))); \
IA ("mov r9 , [rsi+%c0]"::"i"(offsetof(struct Registers , r9))); \
IA ("mov r10 , [rsi+%c0]"::"i"(offsetof(struct Registers , r10))); \
IA ("mov r11 , [rsi+%c0]"::"i"(offsetof(struct Registers , r11))); \
IA ("mov r12 , [rsi+%c0]"::"i"(offsetof(struct Registers , r12))); \
IA ("mov r13 , [rsi+%c0]"::"i"(offsetof(struct Registers , r13))); \
IA ("mov r14 , [rsi+%c0]"::"i"(offsetof(struct Registers , r14))); \
IA ("mov r15 , [rsi+%c0]"::"i"(offsetof(struct Registers , r15))); \
IA ("mov rbp , [rsi+%c0]"::"i"(offsetof(struct Registers , rbp))); \
IA ("push rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , ss)))); \
IA ("mov ss , rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , ds)))); \
IA ("mov ds , rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , es)))); \
IA ("mov es , rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , fs)))); \
IA ("mov fs , rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , gs)))); \
IA ("mov gs , rax"); \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rip))));  \
IA ("mov [rsp+%c0+(8*1)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RIP */ \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , cs)))); \
IA ("mov [rsp+%c0+(8*2)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load CS */ \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rflags)))); \
IA ("mov [rsp+%c0+(8*3)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RFlags */ \
IA ("mov rax , [rsi+%c0]"::"i"((offsetof(struct Registers , rsp)))); \
IA ("mov [rsp+%c0+(8*4)] , rax"::"i"(sizeof(struct Registers)+8));     /* Load RSP */ \
IA ("pop rax"); \
IA ("add rsp , %c0"::"i"(sizeof(struct Registers)+8)); \
IA_INTERRUPT_RETURN

#define INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_FUNCTION(handler_num) \
__attribute__ ((naked)) void interrupt::handler::general_wrapper##handler_num(void) {\
    INTERRUPT_START \
    interrupt_handler_t handler;\
    if((handler = interrupt::general::get_interrupt_handler(handler_num)) == 0x00) {\
        debug::out::printf("Unhandled interrupt %d\n" , handler_num);\
    }\
    else { \
        qword regs_ptr; \
        __asm__ ("mov %0 , rsi":"=r"(regs_ptr)); \
        handler((struct Registers *)regs_ptr); \
    } \
    interrupt::controller::interrupt_received(handler_num);\
    INTERRUPT_END \
}

#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_FUNCTION(handler_num) \
__attribute__ ((naked)) void interrupt::handler::hardware_specified##handler_num(void) {\
    INTERRUPT_START \
    interrupt_handler_t handler;\
    if((handler = interrupt::handler::get_hardware_specified_int_wrapper(handler_num)) == 0x00) {\
        char *name = GLOBAL_OBJECT(interrupt::HardwareSpecifiedInterruptManager)->interrupt_list[handler_num].name;\
        debug::panic("Unhandled special interrupt %d(\"%s\")\n" , handler_num , name);\
    }\
    else { \
        qword regs_ptr; \
        __asm__ ("mov %0 , rsi":"=r"(regs_ptr)); \
        handler((struct Registers *)regs_ptr); \
    }\
    interrupt::controller::interrupt_received(handler_num);\
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