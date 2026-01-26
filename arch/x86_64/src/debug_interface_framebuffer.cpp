#include <kernel/debug.hpp>

#include <string.hpp>

static struct TextScreenInformation {
    max_t vmem;
    
    int width;
    int height;
    int depth;

    int char_width;
    int char_height;

    int font_avg_width;
    int font_avg_height;

    debug_color_t color_background;
    debug_color_t color_foreground;

    int char_x;
    int char_y;
}scrinfo;

struct debug_interface_framebuffer : debug::debug_interface {
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

static void draw_pixel(int x , int y , debug_color_t color) {
    unsigned char *vmem = (unsigned char *)scrinfo.vmem;
    int depth_byte = scrinfo.depth/8;
    int offset = y*(scrinfo.width)+x;
    switch(depth_byte) {
        case 1:
            vmem[offset] = color;
            break;
        case 2:
            vmem[(offset*2)+0] = color & 0xFF;
            vmem[(offset*2)+1] = (color >> 8) & 0xFF;
            break;
        case 3:
            vmem[(offset*3)+0] = color & 0xFF;
            vmem[(offset*3)+1] = (color >> 8) & 0xFF;
            vmem[(offset*3)+2] = (color >> 16) & 0xFF;
            break;
        case 4:
            vmem[(offset*4)+0] = color & 0xFF;
            vmem[(offset*4)+1] = (color >> 8) & 0xFF;
            vmem[(offset*4)+2] = (color >> 16) & 0xFF;

            vmem[(offset*4)+3] = 0x00;
            break;
    }
}

static debug_color_t get_pixel(int x , int y) {
    unsigned char *vmem = (unsigned char *)scrinfo.vmem;
    int depth_byte = scrinfo.depth/8;
    int offset = y*(scrinfo.width)+x;
    switch(depth_byte) {
        case 1: return vmem[offset];
        case 2: return (vmem[(offset*2)+1] << 8)|vmem[(offset*2)+0];
        case 3: return (vmem[(offset*3)+2] << 16)|(vmem[(offset*3)+1] << 8)|vmem[(offset*3)+0];
        case 4: return (vmem[(offset*4)+3] << 24)|(vmem[(offset*4)+2] << 16)|(vmem[(offset*4)+1] << 8)|vmem[(offset*4)+0];
    }
    return 0x00;
}

#include <x86_64/font_consolas.hpp>

static void draw_character(int x , int y , debug_color_t background_color , debug_color_t color , unsigned short character) {
    int padding = 0;
    for(int i = 0; i < consolas_15px_bmp_font_length; i++) {
        if(consolas_15px_bmp_font[i].unicode_id != character) continue;
        int width = consolas_15px_bmp_font[i].width;
        int height = consolas_15px_bmp_font[i].height;
        x += (consolas_20px_bmp_font_avg_width-width)+padding;
        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                int value = consolas_15px_bmp_font[i].bmp_array[h][w];
                if(value == 0) {
                    draw_pixel(x+w , y+h , background_color);
                    continue;
                }
                // value : 0~255, describes the brightness
                double brightness = value/255.0f;
                double r = ((color >> 16) & 0xFF)*brightness;
                double g = ((color >> 8) & 0xFF)*brightness;
                double b = (color & 0xFF)*brightness;
                // yay!
                r += ((background_color >> 16) & 0xFF)*(1-brightness);
                g += ((background_color >> 8) & 0xFF)*(1-brightness);
                b += (background_color & 0xFF)*(1-brightness);

                draw_pixel(x+w , y+h , ((unsigned char)r << 16)|((unsigned char)g << 8)|((unsigned char)b));
            }
        }
        return;
    }
    return;
}

void debug_interface_framebuffer::init(struct LoaderArgument *loader_argument) {
    scrinfo.vmem = (max_t)loader_argument->dbg_graphic_framebuffer_start;
    scrinfo.width = loader_argument->dbg_graphic_framebuffer_width;
    scrinfo.height = loader_argument->dbg_graphic_framebuffer_height;
    scrinfo.depth = loader_argument->dbg_graphic_framebuffer_depth;

    scrinfo.font_avg_width = consolas_15px_bmp_font_avg_width;
    scrinfo.font_avg_height = consolas_15px_bmp_font_avg_height;

    scrinfo.color_foreground = get_color_by_mode(DEBUG_TEXT);
}

void debug_interface_framebuffer::clear_screen(debug_color_t color) {
    for(int h = 0; h < scrinfo.height; h++) {
        for(int w = 0; w < scrinfo.width; w++) {
            draw_pixel(w , h , color);
        }
    }
    set_cursor_position(0 , 0);
    scrinfo.color_background = color;
}

debug_color_t debug_interface_framebuffer::get_color_by_mode(debug_m mode) {
    switch(mode) {
        case DEBUG_NONE:    return 0xDDDDDD;
        case DEBUG_TEXT:    return 0xDDDDDD;
        case DEBUG_INFO:    return 0x6799FF;
        case DEBUG_SPECIAL: return 0x47C83E;
        case DEBUG_ERROR:   return 0xDE3D24;
        case DEBUG_PANIC:   return 0xDE3D24;
        case DEBUG_WARNING: return 0xFFF612;
    }
    return 0xFFFFFF;
}

static void process_newline(void) {
    if((scrinfo.char_y+1)*scrinfo.font_avg_height < scrinfo.height) {
        return;
    }
    int elev = scrinfo.font_avg_height;
    memmove(((void *)scrinfo.vmem) , 
            ((void *)(scrinfo.vmem+(scrinfo.width*scrinfo.font_avg_height*(scrinfo.depth/8)))) , scrinfo.width*(scrinfo.height-elev)*(scrinfo.depth/8));
    
    int depth_byte = scrinfo.depth/8;
    if(depth_byte == 1) {
        for(int off = (scrinfo.height-elev)*scrinfo.width; off < (scrinfo.height-1)*scrinfo.width; off += 1) {
            *((byte *)scrinfo.vmem+off) = scrinfo.color_background;
        }
    }
    if(depth_byte == 2) {
        for(int off = (scrinfo.height-elev)*scrinfo.width*depth_byte; off < (scrinfo.height-1)*scrinfo.width*depth_byte; off += 2) {
            *((word *)(scrinfo.vmem+off)) = (word)(scrinfo.color_background & 0xffff);
        }
    }
    if(depth_byte == 4) {
        for(int off = (scrinfo.height-elev)*scrinfo.width*depth_byte; off < (scrinfo.height-1)*scrinfo.width*depth_byte; off += 4) {
            *((dword *)(scrinfo.vmem+off)) = (dword)(scrinfo.color_background & 0xffffffff);
        }
    }
    if(depth_byte == 8) {
        for(int off = (scrinfo.height-elev)*scrinfo.width*depth_byte; off < (scrinfo.height-1)*scrinfo.width*depth_byte; off += 8) {
            *((qword *)(scrinfo.vmem+off)) = (qword)scrinfo.color_background;
        }
    }
    
    scrinfo.char_y -= 1; 
}

void debug_interface_framebuffer::print_str(const char *str) {
    int real_x = scrinfo.char_x*scrinfo.font_avg_width;
    int real_y = scrinfo.char_y*scrinfo.font_avg_height;
    for(int i = 0; str[i] != 0; i++) {
        if(real_x >= scrinfo.width) {
            scrinfo.char_x = 0;
            scrinfo.char_y += 1;
        }
        real_x = scrinfo.char_x*scrinfo.font_avg_width;
        real_y = scrinfo.char_y*scrinfo.font_avg_height;
        switch(str[i]) {
            case '\n':
                scrinfo.char_x = 0;
                scrinfo.char_y += 1;
                process_newline();
                break;
            case '\b':
                scrinfo.char_x -= 1;
                break;
            case '\r':
                scrinfo.char_x = 0;
                break;
            case '\t':
                scrinfo.char_y += 4;
                break;
            default:
                draw_character(real_x , real_y , scrinfo.color_background , scrinfo.color_foreground , str[i]);
                scrinfo.char_x += 1;
                if((scrinfo.char_x+1)*scrinfo.font_avg_width >= scrinfo.width) {
                    scrinfo.char_x = 0;
                    scrinfo.char_y += 1;
                }
                process_newline();
                break;
        }
    }
}

void debug_interface_framebuffer::set_cursor_position(int x , int y) { scrinfo.char_x = x; scrinfo.char_y = y; }
void debug_interface_framebuffer::get_position_info(int &x , int &y) { x = scrinfo.char_x; y = scrinfo.char_y; }

void debug_interface_framebuffer::move_cursor_position(int x , int y) {
    scrinfo.char_x += x;
    scrinfo.char_y += y;
    if(scrinfo.char_x < 0) {
        scrinfo.char_x = 0;
    }
    if(scrinfo.char_y < 0) {
        scrinfo.char_y = 0;
    }
    if(scrinfo.char_x >= scrinfo.width-1) {
        scrinfo.char_x = scrinfo.width-1;
    }
    if(scrinfo.char_y >= scrinfo.height-1) {
        scrinfo.char_y = scrinfo.height-1;
    }
}

void debug_interface_framebuffer::set_background_color(debug_color_t color) { scrinfo.color_background = color; }
void debug_interface_framebuffer::set_foreground_color(debug_color_t color) { scrinfo.color_foreground = color; }

REGISTER_DEBUG_INTERFACE("framebuffer" , debug_interface_framebuffer)