#include <hardware/interrupt_controller.hpp>
#include <hardware/interrupt_hardware.hpp>
#include <pic.hpp>
#include <interrupt.hpp>

#include <kernel_info.hpp>

#include <debug.hpp>

// use PIC for interrupt controller

void interrupt::controller::init(void) {
    x86_64::pic::init();
}

void interrupt::controller::set_interrupt_mask(int number , bool masked) {
    if(number < 32) return;

    if(masked == true) x86_64::pic::irq_mask(number-32);
    else x86_64::pic::irq_unmask(number-32);
}

void interrupt::controller::disable_all_interrupt(void) {
    x86_64::pic::disable();
}

void interrupt::controller::interrupt_received(void) {
    x86_64::pic::send_eoi_master();
}

void interrupt::controller::interrupt_received(int number) {
    x86_64::pic::send_eoi_master();
    if(number >= 32+8) {
        x86_64::pic::send_eoi_slave();
    }
}

void interrupt::controller::register_hardware_specified_interrupts(void) {
    // nothing for now
}

void interrupt::controller::register_kernel_requested_interrupts(void) {
    GLOBAL_KERNELINFO->kernel_interrupts.timer = 32;
}