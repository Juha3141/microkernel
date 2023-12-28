#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include <interface_type.hpp>
#include <string.hpp>

#define DEBUG_FUNCTION_NAME_LEN  80
#define DEBUG_FUNCTION_STACK_MAX 20

#define DEBUG_NONE      0b000001
#define DEBUG_TEXT      0b000010 // white
#define DEBUG_INFO      0b000100 // blue
#define DEBUG_SPECIAL   0b001000 // light green
#define DEBUG_ERROR     0b010000 // red
#define DEBUG_PANIC     0b100000

#define DEBUG_DISPLAY_FIRST_INFO  0b0000001
#define DEBUG_DISPLAY_FUNCTION    0b0000010
#define DEBUG_DISPLAY_INDENTATION 0b0000100

typedef word debug_m;

namespace debug {
    namespace out {
        void init(void);

        void clear_screen(unsigned char color);
        void vprintf(debug_m mode , const char *fmt , va_list ap);

        void printf(debug_m mode , const char *fmt , ...);
        void printf(const char *fmt , ...);
        void print_str(const char *str);

        void raw_printf(const char *fmt , ...);
    
        void set_position(int x , int y);
        void move_position(int relative_x , int relative_y);
        void get_scr_info(int &x , int &y , unsigned char &background_color , unsigned char &foreground_color);
        unsigned char get_char(int y , int x);
        
        void set_background_color(unsigned char background_color);
        void set_foreground_color(unsigned char foreground_color);

        unsigned char debugcolor(debug_m mode);
        const char *debugstr(debug_m mode);
    }
    void init(void);

    void dump_memory(max_t address , max_t length);

    void push_function(const char *function);
    void pop_function(void);
    
    void set_option(word option , bool flag);

    void display_set(word option);
    void display_mask(word option);

    void enable(void);
    void disable(void);
    void panic(const char *source , int line , const char *fmt , ...);
    void panic(const char *fmt , ...);
    void panic(void);
}

#endif