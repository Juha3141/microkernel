#include <strings.h>
#include <intel_paging.h>

#define KERNEL_STRUCTURE_SIGNATURE 0xC001D00D
#define KERNEL_STRUCTURE_STACKSIZE 8*1024*1024
#define KERNEL_NEW_HIGHER_HALF     0xC0000000

void main(void) {
    
    while(1) {
        ;
    }
}