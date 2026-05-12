#ifndef _REGISTERS_HPP_
#define _REGISTERS_HPP_

// Don't include types.hpp, as types.hpp itself includes registers.hpp!

struct Registers {
    unsigned long long rax;  // 0
    unsigned long long rbx;  // 1
    unsigned long long rcx;  // 2
    unsigned long long rdx;  // 3

    unsigned long long rdi;  // 4
    unsigned long long rsi;  // 5
    
    unsigned long long r8;   // 6
    unsigned long long r9;   // 7
    unsigned long long r10;  // 8
    unsigned long long r11;  // 9
    unsigned long long r12;  // 10
    unsigned long long r13;  // 11
    unsigned long long r14;  // 12
    unsigned long long r15;  // 13

    unsigned long long rbp;  // 14
    unsigned long long rsp;  // 15

    unsigned long long cs;   // 16
    unsigned long long ss;   // 17
    unsigned long long ds;   // 18
    unsigned long long es;   // 19
    unsigned long long fs;   // 20
    unsigned long long gs;   // 21

    unsigned long long rflags; // 22
    unsigned long long rip;    // 23
    
    unsigned long long cr0;    // 24
    unsigned long long cr2;    // 24
    unsigned long long cr3;    // 24

    unsigned long long error_code; // Only used in exception handling
};

#endif