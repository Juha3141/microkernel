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

#include <kernel/interface_type.hpp>
#include <arch/interrupt_hardware.hpp>
#include <arch/interrupt_controller.hpp>
#include <kernel/mem/kmem_manager.hpp>

#include <kernel/interrupt/interrupt_hardware_specified.hpp>

#include <string.hpp>

#define INTERRUPT_TYPE_GENERAL              1
#define INTERRUPT_TYPE_HARDWARE_SPECIFIED   2

#define INTERRUPT_HANDLER_EXCEPTION    0b00000001
#define INTERRUPT_HANDLER_HARDWARE     0b00000010
#define INTERRUPT_HANDLER_SOFTWARE     0b00000100
#define INTERRUPT_HANDLER_LEVEL_KERNEL 0b00001000
#define INTERRUPT_HANDLER_LEVEL_USER   0b00010000

#define KERNEL_REQUEST_TIMER_INTERRUPT 1

namespace interrupt {
    /* interrupt_info_t : Contains information about registering interrupt handler
     * hardware part gives (to kernel) how interrupt is wired through this structure

     * Say, kernel requires interrupt information about Timer interrupt
     * The space for structure that the information is stored is already designated in 
     * Kernel Information structure(struct KernelInfo)
     * What hardware functions have to do is to fill interrupt_info_t structures in the KernelInfo structure.
     * The kernel then will use the information to implement the interrupt system.
     */
    typedef struct interrupt_info_s {
        word type;
        union {
            int number;
            char specified_name[32];
        }location;

        struct interrupt_info_s operator=(int int_number) {
            this->type = INTERRUPT_TYPE_GENERAL;
            this->location.number = int_number;
            return *this;
        }
        struct interrupt_info_s operator=(const char *name) {
            this->type = INTERRUPT_TYPE_HARDWARE_SPECIFIED;
            strcpy(this->location.specified_name , name);
            return *this;
        }
    }interrupt_info_t;
    /* GeneralInterrupt : "number-based" interrupt system (interrupt by interrupt vector) 
     * Most interrupts fit into GeneralInterrupt, but some interrupts that has special circumstance is
     * categorized as "SpecialInterrupt"
     */
    struct GeneralInterrupt {
        interrupt_handler_t handler;
        word option;
    };
    /* SpecialInterrupt : "name-based" interrupt system that don't typically fit in number-based 
     * interrupt categories (such as LAPIC_TIMER .. interrupts that requires some handler)
     */
    struct SpecialInterrupt {
        bool occupied;
        char name[32];
        // Bridge that calls "Assigned"
        interrupt_handler_t assigned_interrupt_handler;
        // Actual interrupt handler that actually does interrupt routine
        interrupt_handler_t interrupt_handler;
        // Deprecated
        // word option;
    };
    /* GeneralInterruptManager : Manages interrupts that has "Interrupt Vector Number"
     * These kinds of interrupt is registered by interrupt vector number and the interrupt handler pointer.
     */
    struct GeneralInterruptManager {
        SINGLETON_PATTERN_KSTRUCT(struct GeneralInterruptManager);
        
        void init(void);
        
        bool register_interrupt(int number , interrupt_handler_t handler , word interrupt_option);
        interrupt_handler_t discard_interrupt(int number);
        
        void mask_interrupt(int number) { mask_flag[number] = true; }
        void unmask_interrupt(int number) { mask_flag[number] = false; }
        bool is_masked(int number) { return mask_flag[number]; }
    
        bool mask_flag[GENERAL_INTERRUPT_MAXCOUNT];
        GeneralInterrupt interrupt_list[GENERAL_INTERRUPT_MAXCOUNT];
    };
    /* HardwareSpecifiedInterruptManager : Manages interrupt that has no interrupt vector number
     * These kinds of interrupt is only registered by 
     */
    
    struct HardwareSpecifiedInterruptManager {
        SINGLETON_PATTERN_KSTRUCT(HardwareSpecifiedInterruptManager);

        void init(int maxcount);

        int register_interrupt_name(const char *name);
        bool discard_interrupt_name(const char *name);

        bool register_kernel_handler(const char *name , ptr_t kernel_handler);
        bool discard_kernel_handler(const char *name);

        interrupt_handler_t register_interrupt_name(const char *name , interrupt_handler_t handler);
            
        int interrupt_maxcount;
        SpecialInterrupt *interrupt_list;
    };

    struct InterruptStackTableManager {
        SINGLETON_PATTERN_KSTRUCT(InterruptStackTableManager);
        max_t ist_location;
        max_t ist_size;
    };
    
    // interrupt_handler_common : 
    void init(void);
    
    /* <General interrupt>
     * The typical interrupt handler that is managed with numeric interrupt vector corresponding to each interrupt.
     */
    namespace general {
        bool register_interrupt(int number , interrupt_handler_t handler , word interrupt_option); // Register interrupt handler
        bool discard_interrupt(int number);                      // Discard interrupt handler
        void set_interrupt_mask(int number , bool masked);
    }
    /* <Hardware-registered interrupt>
     * Similar to kernel-requested interrupt, the hardware can request special handler from kernel that is not
     * able to be registered to "general"(number-based) interrupt system. Known example of this is 
     * LocalAPIC's Timer Interrupt.
     * 
     * Although hardware-requested and kernel-requested interrupt can be collided, but it is highly recommended 
     * to register interrupts to be not collided, as device drivers also have permission to access
     * "hardware-requested" interrupts, and there is potential danger if a device driver occupies kernel-requested
     * interrupt by simply using overlapping hardware-requested interrupt.
     */
    namespace hardware_specified {
        interrupt_handler_t allocate_handler(const char *name);

        bool register_interrupt(const char *name , interrupt_handler_t handler);
        bool discard_interrupt(const char *name);
        void set_interrupt_mask(int number , bool masked);
    }

    // Sorry for bad english lol
    extern "C" void interrupt_handler_common(max_t stack_address , struct SavedRegisters *saved_regs , int int_num);

    bool register_interrupt_by_info(const interrupt_info_t int_info , interrupt_handler_t handler , word option=0x00);
    bool discard_interrupt_by_info(const interrupt_info_t int_info);
}

#endif
