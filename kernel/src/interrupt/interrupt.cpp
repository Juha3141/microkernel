#include <kernel/interrupt/interrupt.hpp>
#include <kernel/io_port.hpp>
#include <kernel/interrupt/predeclared_interrupt_handlers.hpp>

#include <string.hpp>

#include <kernel/debug.hpp>

void interrupt::GeneralInterruptManager::init(void) {
    memset(mask_flag , false , sizeof(mask_flag));
    memset(interrupt_list , 0x00 , sizeof(interrupt_list));
}

bool interrupt::GeneralInterruptManager::register_interrupt(int number , interrupt_handler_t handler , word interrupt_option) {
    // Allow interrupt overriding
    interrupt_list[number].handler = handler;
    interrupt_list[number].option = interrupt_option;
    return true;
}

interrupt_handler_t interrupt::GeneralInterruptManager::discard_interrupt(int number) {
    interrupt_handler_t handler = interrupt_list[number].handler;
    interrupt_list[number].handler = 0x00;
    interrupt_list[number].option = 0x00;
    return handler;
}

#ifdef CONFIG_USE_INTERRUPT

void interrupt::HardwareSpecifiedInterruptManager::init(int maxcount) {
    INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_ARRAY
    interrupt_maxcount = maxcount;
    interrupt_list = (SpecialInterrupt *)memory::pmem_alloc(maxcount*sizeof(SpecialInterrupt));
    for(int i = 0; i < maxcount; i++) {
        interrupt_list[i].occupied = false;
        interrupt_list[i].assigned_interrupt_handler = hardware_specified_wrapper_array[i];
    }
}

interrupt_handler_t interrupt::HardwareSpecifiedInterruptManager::register_interrupt_name(const char *name , interrupt_handler_t handler) {
    INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_ARRAY
    int index = 0;
    for(; index < interrupt_maxcount; index++) {
        if(interrupt_list[index].occupied == false) break;
    }
    if(index >= interrupt_maxcount) {
        debug::out::printf(DEBUG_WARNING , "No available space for new special interrupt\n");
        return 0x00;
    }
    strcpy(interrupt_list[index].name , name);
    interrupt_list[index].occupied = true;
    interrupt_list[index].interrupt_handler = 0x00;
    interrupt_list[index].assigned_interrupt_handler = (interrupt_handler_t)hardware_specified_wrapper_array[index];
    interrupt_list[index].interrupt_handler = handler;
    return interrupt_list[index].assigned_interrupt_handler;
}

bool interrupt::HardwareSpecifiedInterruptManager::discard_interrupt_name(const char *name) {
    int index = 0;
    for(; index < interrupt_maxcount; index++) {
        if(strcmp(interrupt_list[index].name , name) == 0) break;
    }
    if(index >= interrupt_maxcount) {
        return false;
    }
    interrupt_list[index].occupied = false;
    interrupt_list[index].assigned_interrupt_handler = 0x00;
    return true;
}

bool interrupt::HardwareSpecifiedInterruptManager::register_kernel_handler(const char *name , ptr_t kernel_handler) {
    int index = 0;
    for(; index < interrupt_maxcount; index++) {
        if(strcmp(interrupt_list[index].name , name) == 0) break;
    }
    if(index >= interrupt_maxcount) {
        return false;
    }
    if(interrupt_list[index].occupied == false) return false;
    interrupt_list[index].assigned_interrupt_handler = (interrupt_handler_t)kernel_handler;
    return true;
}

bool interrupt::HardwareSpecifiedInterruptManager::discard_kernel_handler(const char *name) {
    int index = 0;
    for(; index < interrupt_maxcount; index++) {
        if(strcmp(interrupt_list[index].name , name) == 0) break;
    }
    if(index >= interrupt_maxcount) {
        return false;
    }
    if(interrupt_list[index].occupied == false) return false;
    interrupt_list[index].assigned_interrupt_handler = 0x00;
    return true;
}

void interrupt::init(void) {
    interrupt::hardware::disable();
    interrupt::hardware::init();
    GLOBAL_OBJECT(GeneralInterruptManager)->init();
    GLOBAL_OBJECT(HardwareSpecifiedInterruptManager)->init(INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_MAXCOUNT);

    interrupt::controller::init();
    interrupt::controller::disable_all_interrupt();
    interrupt::controller::register_hardware_specified_interrupts();
    interrupt::controller::register_kernel_requested_interrupts();
    
#ifdef CONFIG_USE_IST
        GLOBAL_OBJECT(InterruptStackTableManager)->ist_location = 0x00;
        GLOBAL_OBJECT(InterruptStackTableManager)->ist_size = CONFIG_IST_SIZE;
        interrupt::hardware::init_ist();
#endif

}

/////////////////////////////////////////////////////////
//                  general interrupt                  //
/////////////////////////////////////////////////////////

bool interrupt::general::register_interrupt(int number , interrupt_handler_t handler , word interrupt_option , bool wrapper) {
    if(GLOBAL_OBJECT(GeneralInterruptManager)->interrupt_list[number].handler != 0x00) { return false; }
    INTERRUPT_GENERAL_INT_WRAPPER_ARRAY
    ptr_t wrapper_handler = wrapper ? (ptr_t)general_int_wrapper_array[number] : (ptr_t)handler;
    if(interrupt::hardware::register_interrupt(number , wrapper_handler , interrupt_option) == false) return false;
    
    interrupt::controller::set_interrupt_mask(number , false);
    return GLOBAL_OBJECT(GeneralInterruptManager)->register_interrupt(number , handler , interrupt_option);
}

bool interrupt::general::discard_interrupt(int number) {
    if(interrupt::hardware::discard_interrupt(number) == false) return false;
    interrupt::controller::set_interrupt_mask(number , true);
    return GLOBAL_OBJECT(GeneralInterruptManager)->discard_interrupt(number);
}

interrupt_handler_t interrupt::general::get_interrupt_handler(int number) {
    return GLOBAL_OBJECT(GeneralInterruptManager)->interrupt_list[number].handler;
}

void interrupt::general::set_interrupt_mask(int number , bool masked) {
    if(masked == true) {
        GLOBAL_OBJECT(GeneralInterruptManager)->mask_interrupt(number);
    }
    else {
        GLOBAL_OBJECT(GeneralInterruptManager)->unmask_interrupt(number);
    }
    interrupt::controller::set_interrupt_mask(number , masked);
}

/////////////////////////////////////////////////////////
//                kernel_requested part                //
/////////////////////////////////////////////////////////

bool interrupt::register_interrupt_by_info(const interrupt::interrupt_info_t int_info , interrupt_handler_t handler , word interrupt_option) {
    if(int_info.type == INTERRUPT_TYPE_GENERAL) {
        return interrupt::general::register_interrupt(int_info.location.number , handler , interrupt_option);
    }
    if(int_info.type == INTERRUPT_TYPE_HARDWARE_SPECIFIED) {
        return interrupt::hardware_specified::register_interrupt(int_info.location.specified_name , handler);
    }
    return false;
}

bool interrupt::discard_interrupt_by_info(const interrupt::interrupt_info_t int_info) {
    if(int_info.type == INTERRUPT_TYPE_GENERAL) {
        return interrupt::general::discard_interrupt(int_info.location.number);
    }
    if(int_info.type == INTERRUPT_TYPE_HARDWARE_SPECIFIED) {
        return interrupt::hardware_specified::discard_interrupt(int_info.location.specified_name);
    }
    return false;
}

/////////////////////////////////////////////////////////
//               hardware_specified part               //
/////////////////////////////////////////////////////////

interrupt_handler_t interrupt::hardware_specified::allocate_handler(const char *name) {
    return GLOBAL_OBJECT(HardwareSpecifiedInterruptManager)->register_interrupt_name(name , 0x00);
}

bool interrupt::hardware_specified::register_interrupt(const char *name , interrupt_handler_t handler) {
    int index = 0;
    HardwareSpecifiedInterruptManager *hardware_int_mgr = GLOBAL_OBJECT(HardwareSpecifiedInterruptManager);
    for(; index < hardware_int_mgr->interrupt_maxcount; index++) {
        if(strcmp(hardware_int_mgr->interrupt_list[index].name , name) == 0) {
            hardware_int_mgr->interrupt_list[index].interrupt_handler = handler;
            return true;
        }
    }
    return false;
}

bool interrupt::hardware_specified::discard_interrupt(const char *name) {
    return GLOBAL_OBJECT(HardwareSpecifiedInterruptManager)->discard_interrupt_name(name);
}


extern "C" void archindep_general_interrupt_handler(int handler_num , Registers *regs_ptr) {
    interrupt_handler_t handler;
    if((handler = interrupt::general::get_interrupt_handler(handler_num)) == 0x00) {
        debug::panic("Unhandled interrupt invoked, handler_num = %d\n" , handler_num);
    }

    handler(regs_ptr);
    interrupt::controller::interrupt_received(handler_num);
}

extern "C" void archindep_hardware_specified_interrupt_handler(int handler_num , Registers *regs_ptr) {
    interrupt_handler_t handler;
    if((handler = interrupt::handler::get_hardware_specified_int_wrapper(handler_num)) == 0x00) {
        char *name = GLOBAL_OBJECT(interrupt::HardwareSpecifiedInterruptManager)->interrupt_list[handler_num].name;
        debug::panic("Unhandled special interrupt invoked, handler_num = %d, name = \"%s\"\n" , handler_num , name);
    }
    
    handler(regs_ptr);
    interrupt::controller::interrupt_received(handler_num);
}

#else 

void interrupt::HardwareSpecifiedInterruptManager::init(int maxcount) { CONFIG_WARNING_NO_INTERRUPT }
interrupt_handler_t interrupt::HardwareSpecifiedInterruptManager::register_interrupt_name(const char *name , interrupt_handler_t handler) { CONFIG_WARNING_NO_INTERRUPT return 0x00; }
void interrupt::init(void) { interrupt::hardware::disable(); }
bool interrupt::general::register_interrupt(int number , interrupt_handler_t handler , word interrupt_option , bool wrapper) { CONFIG_WARNING_NO_INTERRUPT return false; }
bool interrupt::general::discard_interrupt(int number) { CONFIG_WARNING_NO_INTERRUPT return false; }
void interrupt::general::set_interrupt_mask(int number , bool masked) { CONFIG_WARNING_NO_INTERRUPT }
bool interrupt::register_interrupt_by_info(const interrupt::interrupt_info_t int_info , interrupt_handler_t handler , word interrupt_option) { CONFIG_WARNING_NO_INTERRUPT return false; }
bool interrupt::discard_interrupt_by_info(const interrupt::interrupt_info_t int_info) { CONFIG_WARNING_NO_INTERRUPT return false; }
interrupt_handler_t interrupt::hardware_specified::allocate_handler(const char *name) { CONFIG_WARNING_NO_INTERRUPT return 0x00; }
bool interrupt::hardware_specified::discard_interrupt(const char *name) { CONFIG_WARNING_NO_INTERRUPT return false; }

#endif