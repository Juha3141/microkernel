#include <kernel/debug.hpp>
#include <kernel/io_port.hpp>

struct debug_interface_comport : debug::debug_interface {
    void init(LoaderArgument *loader_argument) override;

    void clear_screen(debug_color_t color) override;
    void print_str(const char *str) override;
        
    void set_cursor_position(int x , int y) override;
    void move_cursor_position(int relative_x , int relative_y) override;
    void get_position_info(int &x , int &y) override;
        
    debug_color_t get_color_by_mode(debug_m mode) override;
    void set_background_color(debug_color_t background_color) override;
    void set_foreground_color(debug_color_t foreground_color) override;

    char interface_identifier[24];
};

void debug_interface_comport::init(LoaderArgument *loader_argument) {
    
}

// clear_screen not supported in serial interface
void debug_interface_comport::clear_screen(debug_color_t color) {}

static void write_string(const char *str) {
    for(int i = 0; str[i] != 0; i++) {
        if(str[i] == '\n') io_write_byte(0x3f8 , '\r');
        io_write_byte(0x3f8 , str[i]);
    }
}

void debug_interface_comport::print_str(const char *str) { write_string(str); }

void debug_interface_comport::set_cursor_position(int x , int y) {}
void debug_interface_comport::move_cursor_position(int relative_x , int relative_y) {}
void debug_interface_comport::get_position_info(int &x , int &y) {}
debug_color_t debug_interface_comport::get_color_by_mode(debug_m mode) { 
    switch(mode) {
        case DEBUG_INFO:
            return 34;
        case DEBUG_PANIC:
        case DEBUG_ERROR:
            return 31;
        case DEBUG_TEXT:
        case DEBUG_NONE:
            return 0;
        case DEBUG_SPECIAL:
            return 32;
        case DEBUG_WARNING:
            return (1<<8)|33;
    }
    return 0;
}

void debug_interface_comport::set_background_color(debug_color_t background_color) {}
void debug_interface_comport::set_foreground_color(debug_color_t foreground_color) {
    if(foreground_color == 0) {
        write_string("\033[0m");
        return;
    }
    char str[50];
    sprintf(str , "\033[%d;%dm" , foreground_color>>8 , foreground_color & 0xff);
    write_string(str);
}

REGISTER_DEBUG_INTERFACE("comport1" , debug_interface_comport);