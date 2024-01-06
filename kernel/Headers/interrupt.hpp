/**
 * @file interrupt.hpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Generalized interrupt manager for global kernel
 * @date 2023-12-30
 * 
 * @copyright Copyright (c) 2023 Ian Juha Cho
 * 
 */

#ifndef _INTERRUPT_HPP_
#define _INTERRUPT_HPP_

#include <interface_type.hpp>
#include <interrupt_hardware.hpp>
#include <kmem_manager.hpp>

#define INTERRUPT_HANDLER_EXCEPTION    0b00000001
#define INTERRUPT_HANDLER_HARDWARE     0b00000010
#define INTERRUPT_HANDLER_SOFTWARE     0b00000100
#define INTERRUPT_HANDLER_LEVEL_KERNEL 0b00001000
#define INTERRUPT_HANDLER_LEVEL_USER   0b00010000

#define INTERRUPT_MAXCOUNT 255

namespace interrupt {
    struct Interrupt {
        ptr_t handler;
        word option;
    };
    // software interrupt manager
    struct InterruptManager {
        void init(void);
        SINGLETON_PATTERN_KSTRUCT(struct InterruptManager);
        
        bool register_interrupt(int number , ptr_t handler , word interrupt_option);
        ptr_t discard_interrupt(int number);
        
        void mask_interrupt(int number) { mask_flag[number] = true; }
        void unmask_interrupt(int number) { mask_flag[number] = false; }
        bool is_masked(int number) { return mask_flag[number]; }
        
        bool mask_flag[INTERRUPT_MAXCOUNT];
        Interrupt interrupt_list[INTERRUPT_MAXCOUNT];
    };
    struct SpecialInterruptManager {
        void init(void);
        SINGLETON_PATTERN_KSTRUCT(struct SpecialInterruptManager);
        
        bool register_interrupt(char *int_name , ptr_t handler , word interrupt_option);
        ptr_t discard_interrupt(char *int_name);
    };
    // interrupt_handler_common : 
    void init(void);
    
    bool register_interrupt(int number , ptr_t handler_ptr , word interrupt_option); // Register interrupt handler
    bool discard_interrupt(int number);                      // Discard interrupt handler
    void set_interrupt_mask(int number , bool masked);

    extern "C" void interrupt_handler_common(max_t stack_address , struct SavedRegisters *saved_regs , int int_num);
}

#endif