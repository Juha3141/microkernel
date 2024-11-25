#include <stdarg.h>

void PrintString(unsigned char color , const char *fmt , ...) {
	static int off=0;
	unsigned char *vmem = (unsigned char  *)0xB8000;
	va_list ap;
	char string[512] = {0 , };
	va_start(ap , fmt);
	vsprintf(string , fmt , ap);
	va_end(ap);
	for(int i = 0; string[i] != 0; i++) {
		switch(string[i]) {
			case '\n':
				off = ((off/80)+(off%80 != 0))*80;
				break;
			default:
				*(vmem+(off*2)) = string[i];
				*(vmem+(off*2)+1) = color;
				off++;
				break;
		}
	}
}