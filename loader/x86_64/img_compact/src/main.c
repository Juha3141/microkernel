#include <strings.h>
#include <intel_paging.h>

#define KERNEL_STRUCTURE_SIGNATURE 0xC001D00D
#define KERNEL_STRUCTURE_STACKSIZE 8*1024*1024
#define KERNEL_NEW_HIGHER_HALF     0xC0000000

void main(void) {
    unsigned char *vmem = (unsigned char *)0xB8000;
    const char string[] = "Hello, world!";
    for(int i = 0; string[i] != 0x00; i++) {
        *(vmem++) = string[i];
        *(vmem++) = 0x01;
    }
    while(1) {
        ;
    }
}