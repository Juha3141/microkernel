#include <kernel/debug.hpp>

#define DEBUG_PRINTF_STR_STACK 1024

struct {
    int function_stack_index;
    char function_stack[DEBUG_FUNCTION_NAME_LEN][DEBUG_FUNCTION_STACK_MAX];

    bool info_display;
    bool function_display;
    bool func_indent;
    bool enable_debug;

    word option_flags;
}debug_info;

void debug::init(KernelArgument *kernel_argument) {
    debug_info.function_stack_index = 0;
    memset(debug_info.function_stack , 0 , sizeof(debug_info.function_stack));
    set_option(DEBUG_DISPLAY_FUNCTION|DEBUG_DISPLAY_FIRST_INFO|DEBUG_DISPLAY_INDENTATION , true);
    debug_info.option_flags = 0b11111111;

    debug::out::init(kernel_argument);
    
    debug::enable();
}

void debug::dump_memory(max_t address , max_t length , bool debug_string) {
    byte *ptr = (byte *)address;
    int margin = 5;
    int char_per_line = 80;
    int x = (char_per_line-margin-3)/4; // character 
    debug::out::printf("dumping memory from 0x%X - 0x%X\n" , address , address+length);
    int line_count = (length/x)+(length%x != 0);
    debug::out::printf("x = %d\n" , x);

    // remove for display
    max_t max = 0;
    max_t offset = 0;
    debug::set_option(DEBUG_DISPLAY_FUNCTION|DEBUG_DISPLAY_INDENTATION , false);
    for(max_t j = 1; j <= line_count; j++) {
        debug::out::printf("");

        unsigned char *old_ptr = ptr;
        max_t count = MIN(x , length-offset);
        for(max_t i = 1; i <= count; i++) {
            debug::out::raw_printf("%02X " , *ptr);
            ptr++;
        }
        max = MAX(max , count);
        if(count < max) {
            for(max_t i = 0; i < max-count; i++) {
                debug::out::raw_printf("   ");
            }
        }
        // debug the string
        debug::out::raw_printf(" | ");
        for(max_t i = 1; i <= count; i++) {
            unsigned char c = *old_ptr;
            switch(c) {
                case '\n':
                case '\t':
                case '\b':
                case '\r':
                case 0:
                    c = '.';
                    break ;
            }
            debug::out::raw_printf("%c" , c);
            old_ptr++;
        }
        debug::out::print_str("\n");
        offset += x;
    }

    debug::set_option(DEBUG_DISPLAY_FUNCTION|DEBUG_DISPLAY_INDENTATION , true);
}

void debug::enable(void) { debug_info.enable_debug = true; }
void debug::disable(void) { debug_info.enable_debug = false; }

void debug::panic_line(const char *source , int line , const char *fmt , ...) {
    va_list ap;
    va_start(ap , fmt);
    char buffer[512];
    vsprintf(buffer , fmt , ap);
    debug::out::printf(DEBUG_PANIC , "(%s:%d) %s", source , line , buffer);
    va_end(ap);
    while(1) {
        ;
    }
}

void debug::panic(const char *fmt , ...) {
    va_list ap;
    va_start(ap , fmt);
    char buffer[512];
    vsprintf(buffer , fmt , ap);
    debug::out::printf(DEBUG_PANIC , buffer);
    va_end(ap);
    while(1) {
        ;
    }
}

void debug::panic(void) {
    debug::out::printf(DEBUG_PANIC , "kernel panic\n");
    while(1) {
        ;
    }
}

const char *debug::out::debugstr(debug_m mode) {
    switch(mode) {
        case DEBUG_NONE:    return "";
        case DEBUG_TEXT:    return "[ ] ";
        case DEBUG_INFO:    return "[I] ";
        case DEBUG_SPECIAL: return "[S] ";
        case DEBUG_WARNING: return "[W] ";
        case DEBUG_ERROR:   return "[E] ";
        case DEBUG_PANIC:   return "[P] ";
    }
    return "    ";
}

void debug::push_function(const char *function) {
    if(debug_info.function_stack_index >= DEBUG_FUNCTION_STACK_MAX) {
        return; // exceeded? just don't add
    }
    // skip, probably a recursive function
    if(strcmp(debug_info.function_stack[debug_info.function_stack_index-1] , function) == 0) {
        return; 
    }
    // different -> add to stack
    strcpy(debug_info.function_stack[debug_info.function_stack_index] , function);
    debug_info.function_stack_index++;
}

void debug::pop_function(void) {
    if(debug_info.function_stack_index <= 0) return;
    debug_info.function_stack_index--;
}

void debug::set_option(word option , bool flag) {
    if((option & DEBUG_DISPLAY_FIRST_INFO) == DEBUG_DISPLAY_FIRST_INFO) {
        debug_info.info_display = flag;
    }
    if((option & DEBUG_DISPLAY_FUNCTION) == DEBUG_DISPLAY_FUNCTION) {
        debug_info.function_display = flag;
    }
    if((option & DEBUG_DISPLAY_INDENTATION) == DEBUG_DISPLAY_INDENTATION) {
        debug_info.func_indent = flag;
    }
}

void debug::display_set(word option) { debug_info.option_flags |= option; }
void debug::display_mask(word option) { debug_info.option_flags &= ~option; }

void debug::out::vprintf(debug_m mode , const char *fmt , va_list ap) {
    if(debug_info.enable_debug == false) return;
    debug_color_t color = debugcolor(mode);
    if((debug_info.option_flags & mode) == 0x00) return; // the mode is set as not to be displayed
    set_foreground_color(color);
    if(debug_info.info_display) {
        print_str(debugstr(mode));
    }
    if(debug_info.function_display && debug_info.function_stack_index > 0) {
        if(debug_info.func_indent) { // indent for debugging the flow of function
            for(int i = 0; i < debug_info.function_stack_index-1; i++) {
                print_str(" ");
            }
        }
        print_str("(");
        print_str(debug_info.function_stack[debug_info.function_stack_index-1]);
        print_str(") ");
    }

    char string[DEBUG_PRINTF_STR_STACK];
    vsprintf(string , fmt , ap);
    print_str(string);

    set_foreground_color(debugcolor(DEBUG_TEXT));
}

void debug::out::printf(debug_m mode , const char *fmt , ...) {
    if(debug_info.enable_debug == false) return;
    va_list ap;
    va_start(ap , fmt);
    debug::out::vprintf(mode , fmt , ap);
    va_end(ap);
}

void debug::out::printf_function(debug_m mode , const char *function , const char *fmt , ...) {
    if(debug_info.enable_debug == false) return;
    va_list ap;
    debug::push_function(function);
    va_start(ap , fmt);
    debug::out::vprintf(mode , fmt , ap);
    va_end(ap);
    debug::pop_function();
}

void debug::out::printf(const char *fmt , ...) {
    if(debug_info.enable_debug == false) return;
    va_list ap;
    va_start(ap , fmt);
    debug::out::vprintf(DEBUG_TEXT , fmt , ap);
    va_end(ap);
}

void debug::out::raw_printf(const char *fmt , ...) {
    if(debug_info.enable_debug == false) return;
    va_list ap;
    va_start(ap , fmt);

    char string[DEBUG_PRINTF_STR_STACK];
    vsprintf(string , fmt , ap);
    print_str(string);
    va_end(ap);
}