#include <kernel/debug.hpp>
#include <string.hpp>

static struct TextScreenInformation {
    unsigned char *vmem;
    int width;
    int height;
    
    int x , y;

    debug_color_t background_color , foreground_color;
}scrinfo;

struct debug_interface_0xb8000 : debug::debug_interface {
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

void debug_interface_0xb8000::set_cursor_position(int x , int y) { scrinfo.x = x; scrinfo.y = y; }
void debug_interface_0xb8000::get_position_info(int &x , int &y) { x = scrinfo.x; y = scrinfo.y; }

void debug_interface_0xb8000::move_cursor_position(int x , int y) {
    scrinfo.x += x;
    scrinfo.y += y;
    if(scrinfo.x < 0) {
        scrinfo.x = 0;
    }
    if(scrinfo.y < 0) {
        scrinfo.y = 0;
    }
    if(scrinfo.x >= scrinfo.width-1) {
        scrinfo.x = scrinfo.width-1;
    }
    if(scrinfo.y >= scrinfo.height-1) {
        scrinfo.y = scrinfo.height-1;
    }
}

void debug_interface_0xb8000::set_background_color(debug_color_t color) { scrinfo.background_color = color; }
void debug_interface_0xb8000::set_foreground_color(debug_color_t color) { scrinfo.foreground_color = color; }

void debug_interface_0xb8000::init(LoaderArgument *loader_argument) {
    scrinfo.vmem = (unsigned char *)loader_argument->dbg_text_framebuffer_start;
    scrinfo.width = loader_argument->dbg_text_framebuffer_width;
    scrinfo.height = loader_argument->dbg_text_framebuffer_height;
}

void debug_interface_0xb8000::clear_screen(debug_color_t color) {
    unsigned char *vmem = (unsigned char *)scrinfo.vmem;
    for(int i = 0; i < scrinfo.width*scrinfo.height*2; i += 2) {
        vmem[i] = 0x00;
        vmem[i+1] = color;
    }
    set_cursor_position(0 , 0);
}

debug_color_t debug_interface_0xb8000::get_color_by_mode(debug_m mode) {
    switch(mode) {
        case DEBUG_NONE:    return 0x07;
        case DEBUG_TEXT:    return 0x07;
        case DEBUG_INFO:    return 0x03;
        case DEBUG_SPECIAL: return 0x02;
        case DEBUG_ERROR:   return 0x04;
        case DEBUG_PANIC:   return 0x04;
        case DEBUG_WARNING: return 0x06;
    }
    return 0x07;
}

static void process_newline(void) {
    if(scrinfo.y > 24) { // fix scrolling problem
        int x = 0 , y = 0;
        int elev = 1;
        for(y = 0; y < scrinfo.height-elev; y++) {
            for(x = 0; x < scrinfo.width; x++) {
                scrinfo.vmem[(y*scrinfo.width+x)*2] = scrinfo.vmem[((y+elev)*scrinfo.width+x)*2];
                scrinfo.vmem[(y*scrinfo.width+x)*2+1] = scrinfo.vmem[((y+elev)*scrinfo.width+x)*2+1];
            }
        }
        for(int y = scrinfo.height-elev; y < scrinfo.height; y++) {
            for(x = 0; x < scrinfo.width; x++) {
                scrinfo.vmem[(y*scrinfo.width+x)*2] = 0x00;
                scrinfo.vmem[(y*scrinfo.width+x)*2+1] = (scrinfo.background_color << 4)|scrinfo.foreground_color;
            }
        }
        scrinfo.y = 24;
    }
}

void debug_interface_0xb8000::print_str(const char *str) {
    int off;
    for(int i = 0; str[i] != 0; i++) {
        off = (scrinfo.y*scrinfo.width*2)+scrinfo.x*2;
        switch(str[i]) {
            case '\n':
                scrinfo.x = 0;
                scrinfo.y++;
                process_newline();
                break;
            case '\b':
                scrinfo.x -= 1;
                if(scrinfo.x < 0) {
                    scrinfo.x = 0;
                }
                scrinfo.vmem[(scrinfo.y*scrinfo.width*2)+scrinfo.x*2] = 0x00;
                break;
            case '\r':
                scrinfo.x = 0;
                break;
            case '\t':
                scrinfo.x += 5;
                break;
            default:
                scrinfo.vmem[off] = str[i];
                scrinfo.vmem[off+1] = (scrinfo.background_color << 4)|scrinfo.foreground_color;
                scrinfo.x++;
                if(scrinfo.x > 79) {
                    scrinfo.x = 0;
                    scrinfo.y++;
                }
                process_newline();
                break;
        }
    }
}

REGISTER_DEBUG_INTERFACE("b8000" , debug_interface_0xb8000)