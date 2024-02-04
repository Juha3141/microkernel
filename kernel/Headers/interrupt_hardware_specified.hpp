#ifndef _INTERRUPT_HARDWARE_SPECIFIED_HPP_
#define _INTERRUPT_HARDWARE_SPECIFIED_HPP_

#include <interface_type.hpp>

#ifdef USE_HARDWARE_INTERRUPT

#define INTERRUPT_HARDWARE_SPECIFIED_MAXCOUNT  32

#define INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(handler_num) interrupt::handler::hardware_specified##handler_num
#define INTERRUPT_HARDWARE_SPECIFIED_HANDLER(handler_num) __attribute__ ((naked)) void hardware_specified##handler_num(void);

#define INTERRUPT_HARDWARE_SPECIFIED_ARRAY \
interrupt_handler_t handlers[INTERRUPT_HARDWARE_SPECIFIED_MAXCOUNT] = {\
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(0) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(1) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(2) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(3) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(4) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(5) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(6) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(7) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(8) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(9) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(10) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(11) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(12) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(13) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(14) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(15) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(16) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(17) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(18) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(19) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(20) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(21) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(22) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(23) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(24) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(25) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(26) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(27) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(28) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(29) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(30) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_HANDLER_PTR(31) \
};

namespace interrupt {
    namespace handler {
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(0);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(1);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(2);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(3);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(4);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(5);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(6);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(7);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(8);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(9);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(10);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(11);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(12);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(13);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(14);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(15);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(16);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(17);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(18);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(19);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(20);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(21);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(22);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(23);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(24);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(25);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(26);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(27);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(28);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(29);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(30);
        INTERRUPT_HARDWARE_SPECIFIED_HANDLER(31);

        interrupt_handler_t get_hardware_specified(int index);
    };
};

#endif

#endif