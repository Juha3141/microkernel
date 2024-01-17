#include <exception.hpp>
#include <exception_handler.hpp>
#include <debug.hpp>
#include <arch_inline_asm.hpp>

#include <hardware/interrupt_hardware.hpp>
#include <hardware/interrupt_controller.hpp>

#define EXCEPTION_HANDLER_FUNCTION(handler_num) \
__attribute__ ((naked)) void exception::handlers::handler##handler_num(void) {\
    exception::global_exception_handler(handler_num); \
    IA_INTERRUPT_RETURN\
}

EXCEPTION_HANDLER_FUNCTION(0)
EXCEPTION_HANDLER_FUNCTION(1)
EXCEPTION_HANDLER_FUNCTION(2)
EXCEPTION_HANDLER_FUNCTION(3)
EXCEPTION_HANDLER_FUNCTION(4)
EXCEPTION_HANDLER_FUNCTION(5)
EXCEPTION_HANDLER_FUNCTION(6)
EXCEPTION_HANDLER_FUNCTION(7)
EXCEPTION_HANDLER_FUNCTION(8)
EXCEPTION_HANDLER_FUNCTION(9)
EXCEPTION_HANDLER_FUNCTION(10)
EXCEPTION_HANDLER_FUNCTION(11)
EXCEPTION_HANDLER_FUNCTION(12)
EXCEPTION_HANDLER_FUNCTION(13)
EXCEPTION_HANDLER_FUNCTION(14)
EXCEPTION_HANDLER_FUNCTION(15)
EXCEPTION_HANDLER_FUNCTION(16)
EXCEPTION_HANDLER_FUNCTION(17)
EXCEPTION_HANDLER_FUNCTION(18)
EXCEPTION_HANDLER_FUNCTION(19)
EXCEPTION_HANDLER_FUNCTION(20)
EXCEPTION_HANDLER_FUNCTION(21)
EXCEPTION_HANDLER_FUNCTION(22)
EXCEPTION_HANDLER_FUNCTION(23)
EXCEPTION_HANDLER_FUNCTION(24)
EXCEPTION_HANDLER_FUNCTION(25)
EXCEPTION_HANDLER_FUNCTION(26)
EXCEPTION_HANDLER_FUNCTION(27)
EXCEPTION_HANDLER_FUNCTION(28)
EXCEPTION_HANDLER_FUNCTION(29)
EXCEPTION_HANDLER_FUNCTION(30)
EXCEPTION_HANDLER_FUNCTION(31)
EXCEPTION_HANDLER_FUNCTION(32)
EXCEPTION_HANDLER_FUNCTION(33)
EXCEPTION_HANDLER_FUNCTION(34)
EXCEPTION_HANDLER_FUNCTION(35)
EXCEPTION_HANDLER_FUNCTION(36)
EXCEPTION_HANDLER_FUNCTION(37)
EXCEPTION_HANDLER_FUNCTION(38)
EXCEPTION_HANDLER_FUNCTION(39)
EXCEPTION_HANDLER_FUNCTION(40)
EXCEPTION_HANDLER_FUNCTION(41)
EXCEPTION_HANDLER_FUNCTION(42)
EXCEPTION_HANDLER_FUNCTION(43)
EXCEPTION_HANDLER_FUNCTION(44)
EXCEPTION_HANDLER_FUNCTION(45)
EXCEPTION_HANDLER_FUNCTION(46)
EXCEPTION_HANDLER_FUNCTION(47)
EXCEPTION_HANDLER_FUNCTION(48)
EXCEPTION_HANDLER_FUNCTION(49)
EXCEPTION_HANDLER_FUNCTION(50)
EXCEPTION_HANDLER_FUNCTION(51)
EXCEPTION_HANDLER_FUNCTION(52)
EXCEPTION_HANDLER_FUNCTION(53)
EXCEPTION_HANDLER_FUNCTION(54)
EXCEPTION_HANDLER_FUNCTION(55)
EXCEPTION_HANDLER_FUNCTION(56)
EXCEPTION_HANDLER_FUNCTION(57)
EXCEPTION_HANDLER_FUNCTION(58)
EXCEPTION_HANDLER_FUNCTION(59)
EXCEPTION_HANDLER_FUNCTION(60)
EXCEPTION_HANDLER_FUNCTION(61)
EXCEPTION_HANDLER_FUNCTION(62)
EXCEPTION_HANDLER_FUNCTION(63)