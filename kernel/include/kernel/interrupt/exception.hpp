/**
 * @file exception.hpp
 * @author Ian Juha Cho(ianisnumber2027@gmail.com)
 * @brief Global kernel exception manager
 * @date 2024-01-15
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */
#ifndef _EXCEPTIONS_HPP_
#define _EXCEPTIONS_HPP_

#include <kernel/essentials.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/mem/kmem_manager.hpp>

#include <object_manager.hpp>

#define EXCEPTIONS_MAXCOUNT 64

#define EXCEPTION_TYPE_GENERAL            1
#define EXCEPTION_TYPE_HARDWARE_SPECIFIC  2
#define EXCEPTION_TYPE_ETC                3

namespace exception {
    namespace hardware {
        /* register_hardware_exceptions : Function for "hardware part," must declare all exceptions */
        void register_hardware_exceptions(void);
    }
    typedef struct exception_info_s {
        word type; // interrupt? hardware_specified interrupt? or etc?
        char name[40];
        ptr_t linked_handler;
        
        interrupt::interrupt_info_t interrupt_info;
    }exception_info_t;
    struct ExceptionManager : FixedArray<exception_info_t> {
        SINGLETON_PATTERN_KSTRUCT(ExceptionManager);

        max_t register_general_int(const char *exception_name , int interrupt_number);
        max_t register_hardware_specified(const char *exception_name , const char *interrupt_name);
        max_t register_etc(const char *exception_name);
        inline void set_handler(int internal_id , ptr_t linked_handler) { (*container[internal_id]).linked_handler = linked_handler; }

        // friend
        friend void global_exception_handler(int handler_id);
    };
    void init(void);

    // Essential service for exception handling
    void register_exception_general_int(const char *exception_name , int general_interrupt_number);
    void register_exception_hardware_specified(const char *exception_name , const char *interrupt_name);
    
    // Miscellaneous
    ptr_t register_exception_etc(const char *exception_name);
    
    void global_exception_handler(int handler_id);
}

#endif