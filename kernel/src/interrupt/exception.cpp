/**
 * @file exception.cpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Global kernel exception manager (source part)
 * @date 2024-01-16
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */
#include <kernel/interrupt/exception.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/interrupt/exception_handler.hpp>

#include <kernel/debug.hpp>

max_t exception::ExceptionManager::register_general_int(const char *exception_name , int general_interrupt_info) {
    max_t id = register_space();

    get_data(id)->type = EXCEPTION_TYPE_GENERAL;
    get_data(id)->interrupt_info = general_interrupt_info;
    strcpy(get_data(id)->name , exception_name);
    return id;
}

max_t exception::ExceptionManager::register_hardware_specified(const char *exception_name , const char *interrupt_name) {
    max_t id = register_space();

    get_data(id)->type = EXCEPTION_TYPE_HARDWARE_SPECIFIC;
    get_data(id)->interrupt_info = interrupt_name;
    strcpy(get_data(id)->name , exception_name);
    return id;
}

max_t exception::ExceptionManager::register_etc(const char *exception_name) {
    max_t id = register_space();

    get_data(id)->type = EXCEPTION_TYPE_ETC;
    strcpy(get_data(id)->name , exception_name);
    return id;
}

#ifdef CONFIG_USE_INTERRUPT

void exception::init(void) {
    GLOBAL_OBJECT(ExceptionManager)->init(EXCEPTIONS_MAXCOUNT);
    exception::hardware::register_hardware_exceptions();
}

void exception::global_exception_handler(int handler_id) {
    ExceptionManager *exception_mgr = ExceptionManager::get_self();
    debug::out::printf(DEBUG_ERROR , "Exception, handler_id : %d, name : %s\n" , handler_id , exception_mgr->get_data(handler_id)->name);
    interrupt::hardware::disable();
    while(1) {
        ;
    }
}

void exception::register_exception_general_int(const char *exception_name , int general_interrupt_number) {
    EXCEPTION_HANDLER_ARRAY
    ExceptionManager *exception_mgr = ExceptionManager::get_self();
    int internal_id = exception_mgr->register_general_int(exception_name , general_interrupt_number);
    if(internal_id == -1) {
        debug::out::printf(DEBUG_WARNING , "No registrable \"general interrupt\" exception\n");
        return; // error
    }
    interrupt_handler_t int_handler = (interrupt_handler_t)handlers[internal_id];
    
    exception_mgr->set_handler(internal_id , handlers[internal_id]);
    interrupt::general::register_interrupt(general_interrupt_number , int_handler , INTERRUPT_HANDLER_EXCEPTION|INTERRUPT_HANDLER_LEVEL_KERNEL , false);
}

void exception::register_exception_hardware_specified(const char *exception_name , const char *interrupt_name) {
    EXCEPTION_HANDLER_ARRAY
    ExceptionManager *exception_mgr = ExceptionManager::get_self();
    int internal_id = exception_mgr->register_hardware_specified(exception_name , interrupt_name);
    if(internal_id == -1) {
        debug::out::printf(DEBUG_WARNING , "No registrable \"hardware-specified interrupt\" exception\n");
        return; // error
    }
    interrupt_handler_t int_handler = (interrupt_handler_t)handlers[internal_id];
    
    exception_mgr->set_handler(internal_id , handlers[internal_id]);
    interrupt::hardware_specified::register_interrupt(interrupt_name , int_handler);
}

ptr_t exception::register_exception_etc(const char *exception_name) {
    EXCEPTION_HANDLER_ARRAY
    ExceptionManager *exception_mgr = ExceptionManager::get_self();
    int internal_id = exception_mgr->register_etc(exception_name);
    if(internal_id == -1) {
        debug::out::printf(DEBUG_WARNING , "No registrable \"etc\" exception\n");
        return 0x00; // error
    }
    return handlers[internal_id];
}

#else

void exception::init(void) { return; }
void exception::register_exception_general_int(const char *exception_name , int general_interrupt_number) { CONFIG_WARNING_NO_INTERRUPT }
void exception::register_exception_hardware_specified(const char *exception_name , const char *interrupt_name) { CONFIG_WARNING_NO_INTERRUPT }
ptr_t exception::register_exception_etc(const char *exception_name) { CONFIG_WARNING_NO_INTERRUPT return 0x00; }
void exception::global_exception_handler(int handler_id) { CONFIG_WARNING_NO_INTERRUPT }

#endif