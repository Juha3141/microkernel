#ifndef _INTERRUPT_CONTROLLER_HPP_
#define _INTERRUPT_CONTROLLER_HPP_

#include <interface_type.hpp>

namespace interrupt {
    namespace controller {
        void init(void);
        void register_hardware_specified_interrupts(void);
        void register_kernel_requested_interrupts(void);

        void set_interrupt_mask(int number , bool masked);

        void interrupt_received(void);        // EOI
        void interrupt_received(int number);

        extern "C" void save_registers(struct SavedRegister *regs);  // Save register routine
        extern "C" void load_registers(struct SavedRegister *regs);  // Load register routine
    }
}

#endif