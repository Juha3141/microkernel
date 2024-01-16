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

#include <interface_type.hpp>
#include <interrupt.hpp>
#include <kmem_manager.hpp>

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
        bool occupied;
        
        word type; // interrupt? hardware_specified interrupt? or etc?
        char name[24];
        ptr_t linked_handler;
        
        interrupt::interrupt_info_t interrupt_info;
    }exception_info_t;
    class ExceptionManager {
        public:
            SINGLETON_PATTERN_KSTRUCT(struct ExceptionManager);

            void init(int exc_maxcount);
            int register_general_int(const char *exception_name , int interrupt_number);
            int register_hardware_specified(const char *exception_name , const char *interrupt_name);
            int register_etc(const char *exception_name);
            inline void set_handler(int internal_id , ptr_t linked_handler) { exception_list[internal_id].linked_handler = linked_handler; }

            // friend
            friend void global_exception_handler(int handler_id);
        private:
            int max_count;
            exception_info_t *exception_list;

            int get_new_list_space(void) {
                int index = 0;
                for(; index < max_count; index++) {
                    if(exception_list[index].occupied == false) {
                        return index;
                    }
                }
                return -1;
            }
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