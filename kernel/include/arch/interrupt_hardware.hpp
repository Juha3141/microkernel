#ifndef _INTERRUPT_HARDWARE_HPP_
#define _INTERRUPT_HARDWARE_HPP_

#include <kernel/essentials.hpp>
#include <registers.hpp>

namespace interrupt {
    namespace hardware {
        void ARCHDEP init(void);   // Initialize interrupt in hardware

        void ARCHDEP init_ist(void);

        extern "C" max_t ARCHDEP get_ist_address(void);

        void ARCHDEP enable(void);
        void ARCHDEP disable(void);
        
        bool ARCHDEP register_interrupt(int number , ptr_t handler_ptr , word interrupt_option); // Register interrupt handler
        bool ARCHDEP discard_interrupt(int number);                      // Discard interrupt handler
        
        extern "C" void ARCHDEP interrupt_entry_save_registers(struct Registers *regs);  // Save register routine
        extern "C" void ARCHDEP interrupt_entry_load_registers(struct Registers *regs);  // Load register routine
    }
}

#endif