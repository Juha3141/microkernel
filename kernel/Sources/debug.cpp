#include <debug.hpp>

struct {
    int function_stack_index;
    char function_stack[DEBUG_FUNCTION_NAME_LEN][DEBUG_FUNCTION_STACK_MAX];

    bool info_display;
    bool function_display;
    bool func_indent;
    bool enable_debug;

    word option_flags;
}debug_info;

void debug::init(void) {
    debug_info.function_stack_index = 0;
    memset(debug_info.function_stack , 0 , sizeof(debug_info.function_stack));
    set_option(DEBUG_DISPLAY_FUNCTION|DEBUG_DISPLAY_FIRST_INFO|DEBUG_DISPLAY_INDENTATION , true);
    debug_info.option_flags = 0b11111111;
    debug::out::init();
}

void debug::enable(void) { debug_info.enable_debug = true; }
void debug::disable(void) { debug_info.enable_debug = false; }

void debug::panic(const char *source , int line , const char *fmt , ...) {
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
    debug::out::printf(DEBUG_PANIC , "%s" , fmt);
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
    unsigned char color = debugcolor(mode);
    if((debug_info.option_flags & mode) == 0x00) return; // the mode is set as not to be displayed
    set_foreground_color(color);
    if(debug_info.info_display) {
        print_str(debugstr(mode));
    }
    if(debug_info.function_display && debug_info.function_stack_index > 0) {
        if(debug_info.func_indent) { // indent for debugging the flow of function
            for(int i = 0; i < debug_info.function_stack_index-1; i++) {
                print_str("-");
            }
        }
        print_str("(");
        print_str(debug_info.function_stack[debug_info.function_stack_index-1]);
        print_str(") ");
    }

    char string[512];
    vsprintf(string , fmt , ap);
    print_str(string);

    set_foreground_color(debugcolor(DEBUG_TEXT));
}


void debug::out::printf(debug_m mode , const char *fmt , ...) {
    va_list ap;
    va_start(ap , fmt);
    debug::out::vprintf(mode , fmt , ap);
    va_end(ap);
}

void debug::out::printf(const char *fmt , ...) {
    va_list ap;
    va_start(ap , fmt);
    debug::out::vprintf(DEBUG_TEXT , fmt , ap);
    va_end(ap);
}

void debug::out::raw_printf(const char *fmt , ...) {
    va_list ap;
    va_start(ap , fmt);

    char string[512];
    vsprintf(string , fmt , ap);
    print_str(string);
    va_end(ap);
}