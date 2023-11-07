#include <debug.hpp>
#include <string.hpp>

static struct TextScreenInformation {
    unsigned char *vmem;
    int width;
    int height;
    unsigned char color_background;
    unsigned char color_foreground;
    int X;
    int Y;
}scrinfo;

void Debug::Initialize(void) {
    scrinfo.vmem = (unsigned char *)0xB8000;
    scrinfo.width = 80;
    scrinfo.height = 25;
    scrinfo.color_background = 0x00;
    scrinfo.color_foreground = 0x07;
}

void Debug::printf(const char *Format , ...) {
    va_list ap;
    char string[256];
    va_start(ap , Format);
    vsprintf(string , Format , ap);
    PrintString(string);
    va_end(ap);
}

void Debug::PrintString(const char *String) {
    int i;
    int j;
    int Offset;
    for(i = 0; String[i] != 0; i++) {
        Offset = (scrinfo.Y*scrinfo.width*2)+scrinfo.X*2;
        switch(String[i]) {
            case '\n':
                scrinfo.X = 0;
                scrinfo.Y++;
                if(scrinfo.Y > 24) {
                    memcpy(scrinfo.vmem , scrinfo.vmem+(scrinfo.width*2) , scrinfo.width*(scrinfo.height-1)*2);
                    for(j = scrinfo.width*(scrinfo.height-1)*2; j < scrinfo.width*scrinfo.height*2; j += 2) {
                        scrinfo.vmem[j] = 0x00;
                        scrinfo.vmem[j+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                    }
                    scrinfo.Y = 24;
                }
                Offset = (scrinfo.Y*scrinfo.width*2)+scrinfo.X*2;
                break;
            case '\b':
                scrinfo.X -= 1;
                if(scrinfo.X < 0) {
                    scrinfo.X = 0;
                }
                scrinfo.vmem[(scrinfo.Y*scrinfo.width*2)+scrinfo.X*2] = 0x00;
                break;
            case '\r':
                scrinfo.X = 0;
                break;
            case '\t':
                scrinfo.X += 5;
                break;
            default:
                scrinfo.vmem[Offset] = String[i];
                scrinfo.vmem[Offset+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                scrinfo.X++;
                if(scrinfo.X > 79) {
                    scrinfo.X = 0;
                    scrinfo.Y++;
                    if(scrinfo.Y > 24) { // fix scrolling problem
                        memcpy(scrinfo.vmem , scrinfo.vmem+(scrinfo.width*2) , scrinfo.width*(scrinfo.height-1)*2);
                        for(j = scrinfo.width*(scrinfo.height-1)*2; j < scrinfo.width*scrinfo.height*2; j += 2) {
                            scrinfo.vmem[j] = 0x00;
                            scrinfo.vmem[j+1] = (scrinfo.color_background << 4)+scrinfo.color_foreground;
                        }
                        scrinfo.Y = 24;
                    }
                    Offset = (scrinfo.Y*scrinfo.width*2)+scrinfo.X*2;
                }
                break;
        }
    }
}

void Debug::SetPosition(int X , int Y) {
    scrinfo.X = X;
    scrinfo.Y = Y;
}

void Debug::MovePosition(int X , int Y) {
    scrinfo.X += X;
    scrinfo.Y += Y;
    if(scrinfo.X < 0) {
        scrinfo.X = 0;
    }
    if(scrinfo.Y < 0) {
        scrinfo.Y = 0;
    }
    if(scrinfo.X >= scrinfo.width-1) {
        scrinfo.X = scrinfo.width-1;
    }
    if(scrinfo.Y >= scrinfo.height-1) {
        scrinfo.Y = scrinfo.height-1;
    }
}

void Debug::GetScreenInformation(int *X , int *Y , unsigned char *BackgroundColor , unsigned char *ForegroundColor) {
    *X = scrinfo.X;
    *Y = scrinfo.Y;
    *BackgroundColor = scrinfo.color_background;
    *ForegroundColor = scrinfo.color_foreground;
}