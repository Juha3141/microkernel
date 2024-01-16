/**
 * @file exception.cpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Global kernel exception manager (source part)
 * @date 2024-01-16
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */
#include <exception.hpp>
#include <interrupt.hpp>
#include <kmem_manager.hpp>
#include <exception_handler.hpp>

#include <debug.hpp>

void exception::ExceptionManager::init(int exc_max_count) {
    this->max_count = exc_max_count;
    this->exception_list = (exception_info_t *)memory::pmem_alloc(sizeof(exception_info_t)*exc_max_count);
    for(int i = 0; i < max_count; i++) {
        exception_list[i].occupied = false;
        memset(&(exception_list[i].interrupt_info) , 0 , sizeof(interrupt::interrupt_info_t));
    }
}

int exception::ExceptionManager::register_general_int(const char *exception_name , int general_interrupt_info) {
    int internal_id = get_new_list_space();
    exception_list[internal_id].occupied = true;
    exception_list[internal_id].type = EXCEPTION_TYPE_GENERAL;
    
    exception_list[internal_id].interrupt_info = general_interrupt_info;
    strcpy(exception_list[internal_id].name , exception_name);
    return internal_id;
}

int exception::ExceptionManager::register_hardware_specified(const char *exception_name , const char *interrupt_name) {
    int internal_id = get_new_list_space();
    exception_list[internal_id].occupied = true;
    exception_list[internal_id].type = EXCEPTION_TYPE_HARDWARE_SPECIFIC;
    
    exception_list[internal_id].interrupt_info = interrupt_name;
    strcpy(exception_list[internal_id].name , exception_name);
    return internal_id;
}

int exception::ExceptionManager::register_etc(const char *exception_name) {
    int internal_id = get_new_list_space();
    exception_list[internal_id].occupied = true;
    exception_list[internal_id].type = EXCEPTION_TYPE_ETC;
    
    strcpy(exception_list[internal_id].name , exception_name);
    return internal_id;
}

void exception::init(void) {
    ExceptionManager::get_self()->init(EXCEPTIONS_MAXCOUNT);
    exception::hardware::register_hardware_exceptions();
}

void exception::global_exception_handler(int handler_id) {
    ExceptionManager *exception_mgr = ExceptionManager::get_self();
    debug::out::printf(DEBUG_ERROR , "Exception, handler_id : %d, name : %s\n" , handler_id , exception_mgr->exception_list[handler_id].name);
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
    interrupt::general::register_interrupt(general_interrupt_number , int_handler , INTERRUPT_HANDLER_EXCEPTION|INTERRUPT_HANDLER_LEVEL_KERNEL);
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