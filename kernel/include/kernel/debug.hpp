#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include <kernel/essentials.hpp>
#include <string.hpp>

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

typedef word debug_m;

typedef dword debug_color_t;

namespace debug {
    namespace out {
        void init(KernelArgument *kernel_argument);

        void clear_screen(debug_color_t color);
        void vprintf(debug_m mode , const char *fmt , va_list ap);

        void printf(debug_m mode , const char *fmt , ...);
        void printf_function(debug_m mode , const char *function , const char *fmt , ...);
        void printf(const char *fmt , ...);
        void print_str(const char *str);

        void raw_printf(const char *fmt , ...);
    
        void set_position(int x , int y);
        void move_position(int relative_x , int relative_y);
        void get_scr_info(int &x , int &y , debug_color_t &background_color , debug_color_t &foreground_color);
        unsigned char get_char(int y , int x);
        
        void set_background_color(debug_color_t background_color);
        void set_foreground_color(debug_color_t foreground_color);

        debug_color_t debugcolor(debug_m mode);
        const char *debugstr(debug_m mode);
    }
    void init(KernelArgument *kernel_argument);

    void dump_memory(max_t address , max_t length , bool debug_string=false);

    void push_function(const char *function);
    void pop_function(void);
    
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