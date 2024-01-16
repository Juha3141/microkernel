#include <interrupt.hpp>
#include <string.hpp>
#include <io_port.hpp>

#include <interrupt_hardware_specified.hpp>

#include <debug.hpp>

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

void interrupt::HardwareSpecifiedInterruptManager::init(int maxcount) {
    INTERRUPT_HARDWARE_SPECIFIED_ARRAY
    interrupt_maxcount = maxcount;
    interrupt_list = (SpecialInterrupt *)memory::pmem_alloc(maxcount*sizeof(SpecialInterrupt));
    for(int i = 0; i < maxcount; i++) {
        interrupt_list[i].occupied = false;
        interrupt_list[i].assigned_interrupt_handler = handlers[i];
    }
}

interrupt_handler_t interrupt::HardwareSpecifiedInterruptManager::register_interrupt_name(const char *name , interrupt_handler_t handler) {
    INTERRUPT_HARDWARE_SPECIFIED_ARRAY
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
    interrupt_list[index].assigned_interrupt_handler = (interrupt_handler_t)handlers[index];
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
    interrupt::hardware::init();
    GeneralInterruptManager::get_self()->init();
    HardwareSpecifiedInterruptManager::get_self()->init(INTERRUPT_HARDWARE_SPECIFIED_MAXCOUNT);
    
    interrupt::controller::init();
    interrupt::controller::register_hardware_specified_interrupts();
    interrupt::controller::register_kernel_requested_interrupts();
}

/////////////////////////////////////////////////////////
//                     general part                    //
/////////////////////////////////////////////////////////

bool interrupt::general::register_interrupt(int number , interrupt_handler_t handler , word interrupt_option) {
    if(interrupt::hardware::register_interrupt(number , (ptr_t)handler , interrupt_option) == false) return false;
    return GeneralInterruptManager::get_self()->register_interrupt(number , handler , interrupt_option);
}

bool interrupt::general::discard_interrupt(int number) {
    if(interrupt::hardware::discard_interrupt(number) == false) return false;
    return GeneralInterruptManager::get_self()->discard_interrupt(number);
}

void interrupt::general::set_interrupt_mask(int number , bool masked) {
    if(masked == true) {
        GeneralInterruptManager::get_self()->mask_interrupt(number);
    }
    else {
        GeneralInterruptManager::get_self()->unmask_interrupt(number);
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
    return HardwareSpecifiedInterruptManager::get_self()->register_interrupt_name(name , 0x00);
}

bool interrupt::hardware_specified::register_interrupt(const char *name , interrupt_handler_t handler) {
    int index = 0;
    HardwareSpecifiedInterruptManager *hardware_int_mgr = HardwareSpecifiedInterruptManager::get_self();
    for(; index < hardware_int_mgr->interrupt_maxcount; index++) {
        if(strcmp(hardware_int_mgr->interrupt_list[index].name , name) == 0) {
            hardware_int_mgr->interrupt_list[index].interrupt_handler = handler;
            return true;
        }
    }
    return false;
}

bool interrupt::hardware_specified::discard_interrupt(const char *name) {
    return HardwareSpecifiedInterruptManager::get_self()->discard_interrupt_name(name);
}