#ifndef _INTERRUPT_CONTROLLER_HPP_
#define _INTERRUPT_CONTROLLER_HPP_

#include <kernel/essentials.hpp>

namespace interrupt {
    namespace controller {
        void ARCHDEP init(void);
        void ARCHDEP register_hardware_specified_interrupts(void);
        void ARCHDEP register_kernel_requested_interrupts(void);

        void ARCHDEP set_interrupt_mask(int number , bool masked);
        void ARCHDEP set_hardware_specified_interrupt_mask();
        void ARCHDEP disable_all_interrupt(void);

        void ARCHDEP interrupt_received(void);        // EOI
        void ARCHDEP interrupt_received(int number);
    }
}

#endif