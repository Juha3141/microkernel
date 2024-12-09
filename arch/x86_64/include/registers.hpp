#ifndef _REGISTERS_HPP_
#define _REGISTERS_HPP_

#include <kernel/types.hpp>

struct Registers {
    qword rax;  // 0
    qword rbx;  // 1
    qword rcx;  // 2
    qword rdx;  // 3

    qword rdi;  // 4
    qword rsi;  // 5
    
    qword r8;   // 6
    qword r9;   // 7
    qword r10;  // 8
    qword r11;  // 9
    qword r12;  // 10
    qword r13;  // 11
    qword r14;  // 12
    qword r15;  // 13

    qword rbp;  // 14
    qword rsp;  // 15

    qword cs;   // 16
    qword ss;   // 17
    qword ds;   // 18
    qword es;   // 19
    qword fs;   // 20
    qword gs;   // 21

    qword rflags; // 22
    qword rip;    // 23
};

#endif