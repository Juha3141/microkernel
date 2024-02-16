#include <debug.hpp>
#include <string.hpp>

static struct TextScreenInformation {
    unsigned char *vmem;
    int width;
    int height;
    unsigned char color_background;
    unsigned char color_foreground;
    int x;
    int y;
}scrinfo;

void debug::out::init(void) {
    scrinfo.vmem = (unsigned char *)0xB8000;
    scrinfo.width = 80;
    scrinfo.height = 25;
    scrinfo.color_background = 0x00;
    scrinfo.color_foreground = 0x07;
}

void debug::out::clear_screen(unsigned char color) {
    unsigned char *vmem = (unsigned char *)scrinfo.vmem;
    for(int i = 0; i < scrinfo.width*scrinfo.height*2; i += 2) {
        vmem[i] = ' ';
        vmem[i+1] = color;
    }
    set_position(0 , 0);
}

unsigned char debug::out::debugcolor(debug_m mode) {
    switch(mode) {
        case DEBUG_NONE:    return 0x07;
        case DEBUG_TEXT:    return 0x07;
        case DEBUG_INFO:    return 0x03;
        case DEBUG_SPECIAL: return 0x02;
        case DEBUG_ERROR:   return 0x04;
        case DEBUG_PANIC:   return 0x04;
        case DEBUG_WARNING: return 0x08;
    }
    return 0x07;
}

void debug::out::print_str(const char *str) {
    int i;
    int j;
    int off;
    for(i = 0; str[i] != 0; i++) {
        off = (scrinfo.y*scrinfo.width*2)+scrinfo.x*2;
        switch(str[i]) {
            case '\n':
                scrinfo.x = 0;
                scrinfo.y++;
                if(scrinfo.y > 24) {
                    memcpy(scrinfo.vmem , scrinfo.vmem+(scrinfo.width*2) , scrinfo.width*(scrinfo.height-1)*2);
                    for(j = scrinfo.width*(scrinfo.height-1)*2; j < scrinfo.width*scrinfo.height*2; j += 2) {
                        scrinfo.vmem[j] = 0x00;
                        scrinfo.vmem[j+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                    }
                    scrinfo.y = 24;
                }
                off = (scrinfo.y*scrinfo.width*2)+scrinfo.x*2;
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
                scrinfo.vmem[off+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                scrinfo.x++;
                if(scrinfo.x > 79) {
                    scrinfo.x = 0;
                    scrinfo.y++;
                    if(scrinfo.y > 24) { // fix scrolling problem
                        memcpy(scrinfo.vmem , scrinfo.vmem+(scrinfo.width*2) , scrinfo.width*(scrinfo.height-1)*2);
                        for(j = scrinfo.width*(scrinfo.height-1)*2; j < scrinfo.width*scrinfo.height*2; j += 2) {
                            scrinfo.vmem[j] = 0x00;
                            scrinfo.vmem[j+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                        }
                        scrinfo.y = 25;
                    }
                    off = (scrinfo.y*scrinfo.width*2)+scrinfo.x*2;
                }
                break;
        }
    }
}


void debug::out::set_position(int x , int y) {
    scrinfo.x = x;
    scrinfo.y = y;
}

void debug::out::move_position(int x , int y) {
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

void debug::out::get_scr_info(int &x , int &y , unsigned char &background_color , unsigned char &foreground_color) {
    x = scrinfo.x;
    x = scrinfo.y;
    background_color = scrinfo.color_background;
    foreground_color = scrinfo.color_foreground;
}

void debug::out::set_background_color(unsigned char background_color) {
    scrinfo.color_background = background_color;
}

void debug::out::set_foreground_color(unsigned char foreground_color) {
    scrinfo.color_foreground = foreground_color;
}