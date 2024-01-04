#include <interrupt.hpp>
#include <string.hpp>

#include <debug.hpp>

void interrupt::InterruptManager::init(void) {
    memset(mask_flag , false , sizeof(mask_flag));
    memset(interrupt_list , 0x00 , sizeof(interrupt_list));
}

bool interrupt::InterruptManager::register_interrupt(int number , ptr_t handler , word interrupt_option) {
    if(interrupt_list[number].handler != 0x00) {
        return false;
    }
    interrupt_list[number].handler = handler;
    interrupt_list[number].option = interrupt_option;
    return true;
}

ptr_t interrupt::InterruptManager::discard_interrupt(int number) {
    ptr_t handler = interrupt_list[number].handler;
    interrupt_list[number].handler = 0x00;
    interrupt_list[number].option = 0x00;
    return handler;
}

void interrupt::init(void) {
    InterruptManager::get_self()->init();
    hardware::init();
}

bool interrupt::register_interrupt(int number , ptr_t handler_ptr , word interrupt_option) {
    if(hardware::register_interrupt(number , handler_ptr , interrupt_option) == false) return false;
    return InterruptManager::get_self()->register_interrupt(number , handler_ptr , interrupt_option);
}

bool interrupt::discard_interrupt(int number) {
    if(hardware::discard_interrupt(number) == false) return false;
    return InterruptManager::get_self()->discard_interrupt(number);
}

bool interrupt::mask_interrupt(int number) {
    if(hardware::mask_interrupt(number) == false) return false;
    InterruptManager::get_self()->mask_interrupt(number);
    return true;
}

bool interrupt::unmask_interrupt(int number) {
    if(hardware::unmask_interrupt(number) == false) return false;
    InterruptManager::get_self()->unmask_interrupt(number);
    return true;
}