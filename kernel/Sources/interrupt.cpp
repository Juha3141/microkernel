#include <interrupt.hpp>
#include <string.hpp>
#include <io_port.hpp>

#include <debug.hpp>

void interrupt::InterruptManager::init(void) {
    memset(mask_flag , false , sizeof(mask_flag));
    memset(interrupt_list , 0x00 , sizeof(interrupt_list));
}

bool interrupt::InterruptManager::register_interrupt(int number , ptr_t handler , word interrupt_option) {
    // Allow interrupt overriding
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

/*

__attribute__ ((naked)) void uninitialized_interrupt_handler_8(void) {
    debug::push_function("fucktest");
    debug::out::printf(DEBUG_ERROR , "Double Fault #8\n");
    debug::pop_function();
    
    __asm__ ("iretq");
}

__attribute__ ((naked)) void uninitialized_interrupt_handler_13(void) {
    debug::push_function("fucktest");
    debug::panic("General Protection Fault #13\n");
    while(1) {
        ;
    }
}

__attribute__ ((naked)) void uninitialized_interrupt_handler_32(void) {
    io_write_byte(0x20 , 0x20);
    __asm__ ("iretq");
}

*/

void interrupt::init(void) {
    InterruptManager::get_self()->init();
    interrupt::hardware::init();
    // To-do : Get some handlers
}

bool interrupt::register_interrupt(int number , ptr_t handler_ptr , word interrupt_option) {
    if(hardware::register_interrupt(number , handler_ptr , interrupt_option) == false) return false;
    return InterruptManager::get_self()->register_interrupt(number , handler_ptr , interrupt_option);
}

bool interrupt::discard_interrupt(int number) {
    if(hardware::discard_interrupt(number) == false) return false;
    return InterruptManager::get_self()->discard_interrupt(number);
}

void interrupt::set_interrupt_mask(int number , bool masked) {
    if(masked == true) {
        InterruptManager::get_self()->mask_interrupt(number);
    }
    else {
        InterruptManager::get_self()->unmask_interrupt(number);
    }
    interrupt::hardware::set_interrupt_mask(number , masked);
}