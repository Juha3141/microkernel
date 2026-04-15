#include <kernel/interrupt/exception.hpp>

void exception::hardware::register_hardware_exceptions(void) {
    exception::register_exception_general_int("Divided by zero" , 0);
    exception::register_exception_general_int("Debug" , 1);
    exception::register_exception_general_int("Non-maskable Interrupt" , 2);
    exception::register_exception_general_int("Breakout" , 3);
    exception::register_exception_general_int("Overflow" , 4);
    exception::register_exception_general_int("Bound Range Exceeded" , 5);
    exception::register_exception_general_int("Invalid OpCode" , 6);
    exception::register_exception_general_int("Device Not Available" , 7);
    exception::register_exception_general_int("Double Fault" , 8);
    exception::register_exception_general_int("Coprocessor Segment Overrun" , 9);
    exception::register_exception_general_int("Invalid TSS" , 10);
    exception::register_exception_general_int("Segment Not Present" , 11);
    exception::register_exception_general_int("Stack Segment Fault" , 12);
    exception::register_exception_general_int("General Protection Fault" , 13);
    exception::register_exception_general_int("Page Fault" , 14);
    exception::register_exception_general_int("Reserved" , 15);
    exception::register_exception_general_int("x87 Floating-point Exception" , 16);
    exception::register_exception_general_int("Alignment Check" , 17);
    exception::register_exception_general_int("Machine Check" , 18);
    exception::register_exception_general_int("SIMD Floating-point Exception" , 19);
    exception::register_exception_general_int("Virtualization Exception" , 20);
    exception::register_exception_general_int("Control Protection Exception" , 21);
    exception::register_exception_general_int("Hypervisor Injection Exception" , 28);
    exception::register_exception_general_int("VMM Communication Exception" , 29);
    exception::register_exception_general_int("Security Exception" , 30);

    for(int i = 22; i <= 27; i++) {
        exception::register_exception_general_int("Reserved" , i);
    }
    exception::register_exception_general_int("Reserved" , 31);
}

void exception::hardware::archdep_general_exception_handler(int handler_id , Registers *regs) {
    debug::out::printf(DEBUG_ERROR , "RAX=0x%-16llx RBX=0x%-16llx RCX=0x%-16llx RDX=0x%-16llx\n" , regs->rax,regs->rbx,regs->rcx,regs->rdx);
    debug::out::printf(DEBUG_ERROR , "RDI=0x%-16llx RSI=0x%-16llx RBP=0x%-16llx RSP=0x%-16llx\n" , regs->rdi,regs->rsi,regs->rbp,regs->rsp);
    debug::out::printf(DEBUG_ERROR , "R8 =0x%-16llx R9 =0x%-16llx R10=0x%-16llx R11=0x%-16llx\n" , regs->r8,regs->r9,regs->r10,regs->r11);
    debug::out::printf(DEBUG_ERROR , "R12=0x%-16llx R13=0x%-16llx R14=0x%-16llx R15=0x%-16llx\n" , regs->r12,regs->r13,regs->r14,regs->r15);
    debug::out::printf(DEBUG_ERROR , "RIP=0x%-16llx\n" , regs->rip);
    debug::out::printf(DEBUG_ERROR , "CR0=0x%-16llx CR2=0x%-16llx CR3=0x%-16llx\n" , regs->cr0,regs->cr2,regs->cr3);
}