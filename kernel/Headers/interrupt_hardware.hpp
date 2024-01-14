#ifndef _INTERRUPT_HARDWARE_HPP_
#define _INTERRUPT_HARDWARE_HPP_

#include <interface_type.hpp>
#include <interrupt_specification.hpp>

namespace interrupt {
    namespace hardware {
        void init(void);   // Initialize interrupt in hardware
        
        void enable(void);
        void disable(void);
        
        bool register_interrupt(int number , ptr_t handler_ptr , word interrupt_option); // Register interrupt handler
        bool discard_interrupt(int number);                      // Discard interrupt handler
    }
}

namespace exception {
    
};

#endif