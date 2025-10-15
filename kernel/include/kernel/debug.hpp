#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include <kernel/essentials.hpp>
#include <loader/loader_argument.hpp>
#include <object_manager.hpp>
#include <string.hpp>
#include <linked_list.hpp>

#define DEBUG_FUNCTION_NAME_LEN  40
#define DEBUG_FUNCTION_STACK_MAX 20

#define DEBUG_NONE      0b0000001
#define DEBUG_TEXT      0b0000010 // white
#define DEBUG_INFO      0b0000100 // blue
#define DEBUG_SPECIAL   0b0001000 // light green
#define DEBUG_WARNING   0b0010000 // yellow
#define DEBUG_ERROR     0b0100000 // red
#define DEBUG_PANIC     0b1000000 // red

#define DEBUG_DISPLAY_FIRST_INFO  0b0000001
#define DEBUG_DISPLAY_FUNCTION    0b0000010
#define DEBUG_DISPLAY_INDENTATION 0b0000100

#define DEBUG_STOP while(1) {}

typedef void(*debug_interface_init_func_ptr_t)(void);
#define REGISTER_DEBUG_INTERFACE(name , interface_class) void register_debug_interface_##interface_class(void) { debug::register_debug_interface(new interface_class , name); } \
static __attribute__ ((section(".debug_interface"))) debug_interface_init_func_ptr_t __debug_interface_init_##interface_class = register_debug_interface_##interface_class;

typedef word debug_m;

typedef dword debug_color_t;

namespace debug {
    struct debug_interface {
        virtual void init(LoaderArgument *loader_argument) = 0;

        virtual void clear_screen(debug_color_t color) = 0;
        virtual void print_str(const char *str) = 0;
        
        virtual void set_cursor_position(int x , int y) = 0;
        virtual void move_cursor_position(int relative_x , int relative_y) = 0;
        virtual void get_position_info(int &x , int &y) = 0;
        
        virtual debug_color_t get_color_by_mode(debug_m mode) = 0;
        virtual void set_background_color(debug_color_t background_color) = 0;
        virtual void set_foreground_color(debug_color_t foreground_color) = 0;
        virtual debug_color_t get_background_color(void) = 0;
        virtual debug_color_t get_foreground_color(void) = 0;

        char interface_identifier[24];
    };
    struct debug_message_t {
        int log_level;
        char *debug_msg;
    };
    struct DebugMessageContainer {
        void init(void) {
            
        }
        
        LinkedList<debug_message_t>message_list;
    };

    struct DebugInterfaceContainer {
        void init(int max_obj_count) {
            // this crashes somehow
            object_container = (struct object_container_s *)memory::kstruct_alloc(sizeof(struct object_container_s)*max_obj_count);
            for(int i = 0; i < max_obj_count; i++) {
                object_container[i].occupied = false;
                object_container[i].object = 0x00;
            }

            max_count = max_obj_count;
            count = 0;
        }
        max_t register_object(debug_interface *object) { // returns ID
            max_t i;
            if(object_container == 0x00) {
                // debug::out::printf(DEBUG_WARNING , "Error : ObjectContainer yet not initialized\n");
                return INVALID;
            }
            if(count >= max_count) return INVALID;
            for(i = 0; i < max_count; i++) {
                if(object_container[i].occupied == false) {
                    break;
                }
            }
            if(i >= max_count) return INVALID;
            object_container[i].occupied = true;
            object_container[i].object = object;
            count++;
            return i;
        }
        max_t discard_object(debug_interface *object) {
            max_t i;
            for(i = 0; i < max_count; i++) {
                if(object_container[i].object == object) {
                    count--;
                    object_container[i].occupied = false;
                    return i;
                }
            }
            return INVALID;
        }
        debug_interface *get_object(max_t id) {
            if((id >= max_count)||(object_container[id].occupied == false)) return 0x00;
            return object_container[id].object;
        }
        
        template <typename T2> max_t search(bool (*check)(debug_interface *data , T2 sample_data) , T2 sample_data) {
            for(max_t i = 0; i < max_count; i++) {
                if(check(object_container[i].object , sample_data) == true) {
                    return i;
                }
            }
            return INVALID;
        }
        max_t max_count;
        max_t count;
        struct object_container_s {
            bool occupied;
            debug_interface *object;
        }*object_container;
    };

    namespace out {
        void vprintf(debug_m mode , const char *fmt , va_list ap);

        void printf(debug_m mode , const char *fmt , ...);
        void printf(const char *fmt , ...);

        void raw_printf(const char *fmt , ...);

        // debug_interface functions
        void clear_screen(debug_color_t color);
        
        void print_str(const char *str);
        void set_cursor_position(int x , int y);
        void move_cursor_position(int relative_x , int relative_y);
        void get_position_info(int &x , int &y);
        
        void set_background_color(debug_color_t background_color);
        void set_foreground_color(debug_color_t foreground_color);
        debug_color_t get_background_color(void);
        debug_color_t get_foreground_color(void);

        const char *debugstr(debug_m mode);
    }
    void init(LoaderArgument *kernel_argument);
    void register_debug_interface(debug_interface *interface , const char *interface_identifier);
    void register_debug_interface(debug_interface *interface);
    void set_current_debug_interface(const char *interface_identifier);
    debug_interface *current_debug_interface(void);

    void dump_memory(max_t address , max_t length , bool debug_string=false);
    
    void set_option(word option , bool flag);

    void display_set(word option);
    void display_mask(word option);

    void enable(void);
    void disable(void);

    void panic_line(const char *source , int line , const char *fmt , ...);
    void panic(const char *fmt , ...);
    void panic(void);
}

#endif