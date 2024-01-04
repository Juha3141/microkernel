/**
 * @file interrupt_hardware.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The hardware part of interrupt manager. Contains the Intel Interrupt Descriptor Table manager
 * @date 2023-12-31
 * 
 * @copyright Copyright (c) 2023 Ian Juha Ch
 * 
 */

#include <interrupt.hpp>
#include <interrupt_hardware.hpp>
#include <interrupt_specification.hpp>
#include <idt.hpp>
#include <pic.hpp>
#include <arch_inline_asm.hpp>

#include <string.hpp>
#include <debug.hpp>

void interrupt::hardware::enable(void) {
    __asm__ ("sti");
}

void interrupt::hardware::disable(void) {
    __asm__ ("cli");
}

void interrupt::hardware::init(void) {
    interrupt::hardware::disable();
    x86_64::pic::init();

    debug::push_function("int::hw::init");

    x86_64::IDTContainer *idt_container = x86_64::IDTContainer::get_self();
    idt_container->init(IDT_ENTRYCOUNT);
    // Register IDTR
    max_t idtr_ptr = (max_t)&idt_container->reg;
    debug::out::printf("idtr_ptr      : 0x%X\n" , idtr_ptr);
    debug::out::printf("idt base_addr : 0x%X\n" , idt_container->entries);
    // IA ("lidt [%0]":"=r"(idtr_ptr));
    
    debug::pop_function();
}

bool interrupt::hardware::register_interrupt(int number , ptr_t handler_ptr , word interrupt_option) {
    word flags = 0;
    word privilege = 0;
    x86_64::IDTContainer *idt_container = x86_64::IDTContainer::get_self();
    if(number >= INTERRUPT_MAXCOUNT) return false;
    idt_container->entries[number].base_low = handler_ptr & 0xFFFF;
    idt_container->entries[number].base_high = handler_ptr >> 16;
    if((interrupt_option & INTERRUPT_HANDLER_EXCEPTION) == INTERRUPT_HANDLER_EXCEPTION) flags |= IDT_TYPE_32BIT_TRAP_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_HARDWARE) == INTERRUPT_HANDLER_HARDWARE)   flags |= IDT_TYPE_32BIT_INTERRUPT_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_SOFTWARE) == INTERRUPT_HANDLER_SOFTWARE)   flags |= IDT_TYPE_32BIT_INTERRUPT_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_LEVEL_KERNEL) == INTERRUPT_HANDLER_LEVEL_KERNEL) privilege |= IDT_FLAGS_DPL0;
    if((interrupt_option & INTERRUPT_HANDLER_LEVEL_USER) == INTERRUPT_HANDLER_LEVEL_USER)     privilege |= IDT_FLAGS_DPL3;
    idt_container->entries[number].flags = flags;
    idt_container->entries[number].IST = 0;
    idt_container->entries[number].type = privilege|IDT_FLAGS_P;
    idt_container->entries[number].selector = 0x08; // !!!
    idt_container->entries[number].reserved = 0x00;
    return true;
}

bool interrupt::hardware::discard_interrupt(int number) {
    x86_64::IDTContainer *idt_container = x86_64::IDTContainer::get_self();
    if(number >= INTERRUPT_MAXCOUNT) return false;
    memset(&(idt_container->entries[number]) , 0 , sizeof(x86_64::IDTEntry));
    return true;
}

bool interrupt::hardware::mask_interrupt(int number) {
    return true;
}

bool interrupt::hardware::unmask_interrupt(int number) {
    return false;
}

void interrupt::hardware::interrupt_received(void) {
    // x86_64::pic::send_EOI();
}