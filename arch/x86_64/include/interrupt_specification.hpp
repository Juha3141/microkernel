#ifndef _INTERRUPT_SPECIFICATION_HPP_
#define _INTERRUPT_SPECIFICATION_HPP_

struct SavedRegisters {
    unsigned long RAX;
    unsigned long RBX;
    unsigned long RCX;
    unsigned long RDX;

    unsigned long RDI;
    unsigned long RSI;
    
    unsigned long RBP;
    unsigned long RSP;

    unsigned long R8;
    unsigned long R9;
    unsigned long R10;
    unsigned long R11;
    unsigned long R12;
    unsigned long R13;
    unsigned long R14;
    unsigned long R15;
};

#endif