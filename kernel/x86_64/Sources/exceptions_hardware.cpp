#include <exception.hpp>

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