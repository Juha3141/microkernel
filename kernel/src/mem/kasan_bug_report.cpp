#include <kernel/mem/kasan.hpp>
#include <kernel/debug.hpp>

#include <kernel/mem/nodes_manager.hpp>
#include <kernel/interrupt/interrupt.hpp>

extern char __kernel_main_start__;
extern char __kernel_main_end__;

#define DUMP_STACK_LVL_SAFE(N) \
    debug::out::printf(DEBUG_WARNING , "      %d. pc=0x%llx\n" , N , __builtin_return_address(N)); \
    if(__builtin_return_address(N) >= &__kernel_main_start__ && __builtin_return_address(N) <= &__kernel_main_end__) return; \

static inline void dump_stack_until_main(void) {
    DUMP_STACK_LVL_SAFE(3)
    DUMP_STACK_LVL_SAFE(4)
    DUMP_STACK_LVL_SAFE(5)
    DUMP_STACK_LVL_SAFE(6)
    DUMP_STACK_LVL_SAFE(7)
    DUMP_STACK_LVL_SAFE(8)
    DUMP_STACK_LVL_SAFE(9)
    DUMP_STACK_LVL_SAFE(10)
    DUMP_STACK_LVL_SAFE(11)
    DUMP_STACK_LVL_SAFE(12)
    DUMP_STACK_LVL_SAFE(13)
}

static inline const char *shadow_byte_to_str(byte b) {
    switch(b) {
        case KASAN_SHADOW_MAGIC_HEAP_FREE:          return "Heap-Use-After-Free";
        case KASAN_SHADOW_MAGIC_HEAP_HEAD_REDZONE:  return "Heap-Head-Redzone";
        case KASAN_SHADOW_MAGIC_HEAP_TAIL_REDZONE:  return "Heap-Tail-Redzone";
        case KASAN_SHADOW_MAGIC_STACK_AFTER_RETURN: return "Stack-After-Return";
        case KASAN_SHADOW_MAGIC_STACK_AFTER_SCOPE:  return "Stack-After-Scope";
        case KASAN_SHADOW_MAGIC_GLOBAL_REDZONE:     return "Global-Reserved";
        case KASAN_SHADOW_MAGIC_RESERVED:           return "Reserved";
    }
    return "Unknown";
}

__no_sanitize_address__
unsigned long get_poisoned_shadow_address(max_t linear_address , max_t size) {
    if(!kasan::is_enabled()) return 0;

    max_t shadow_start_addr = KASAN_LADDR_TO_SHADOW(linear_address);
    max_t shadow_end_addr   = KASAN_LADDR_TO_SHADOW(linear_address+size-1)+1;
    max_t nonzero_shadow_addr = 0;

    for(max_t addr = shadow_start_addr; addr < shadow_end_addr; addr++) {
        if(*(byte*)addr != KASAN_SHADOW_MAGIC_UNPOISONED) {
            nonzero_shadow_addr = addr;
            break;
        }
    }
    if(nonzero_shadow_addr) {
        max_t last_byte = linear_address+size-1;
        char *last_shadow_byte = (char *)KASAN_LADDR_TO_SHADOW(last_byte);

        if(nonzero_shadow_addr != (max_t)last_shadow_byte
        || ((char)(last_byte & KASAN_SHADOW_MASK)) >= *last_shadow_byte) {
            return nonzero_shadow_addr;
        }
    }
    return 0;
}

__no_sanitize_address__
void kasan::report_bug(max_t addr , max_t size , byte is_write , max_t pc , bool noabort) {
    debug::out::printf(DEBUG_ERROR , "------------- kasan::report_bug() -------------\n");
    debug::out::printf(DEBUG_ERROR , "From trying to access 0x%llx sz=%d   " , addr , size);
    
    debug::out::printf(DEBUG_ERROR , "(%s)\n"  , is_write ? "Write" : "Read");
    // debug::out::printf(DEBUG_ERROR , "Type : %s\n" , shadow_byte_to_str(*((byte *)buggy_shadow_address)));

    max_t buggy_shadow_address = get_poisoned_shadow_address(addr , size);
    max_t exact_poisoned_address = KASAN_SHADOW_TO_LADDR(buggy_shadow_address);

    debug::out::printf(DEBUG_ERROR , "Exact location : 0x%llx\n" , exact_poisoned_address);
    debug::out::printf(DEBUG_ERROR , "Shadow address : 0x%llx\n" , buggy_shadow_address);
    debug::out::printf(DEBUG_ERROR , "Error type     : %s(%02x)\n" , shadow_byte_to_str(*((byte *)buggy_shadow_address)) , *((byte *)buggy_shadow_address));

    debug::out::printf(DEBUG_ERROR , "------ Stack trace : \n");
    dump_stack_until_main();
    debug::out::printf(DEBUG_ERROR , "------ Shadow memory : \n");
    max_t shadow_size = align_round_up(size , KASAN_GRANUL_SIZE) >> KASAN_SHADOW_SHIFT;
    max_t shadow_area_dump_start = buggy_shadow_address-16;
    max_t shadow_area_dump_end   = buggy_shadow_address+16+shadow_size;
    max_t range = align_round_up(shadow_area_dump_end-shadow_area_dump_start , 8);

    debug::out::printf("0x%llx : " , shadow_area_dump_start);
    for(max_t i = 0; i < range; i++) {
        if(i%8 == 0 && i > 0) debug::out::printf("\n0x%llx : " , shadow_area_dump_start+i);
        byte b = *((byte *)shadow_area_dump_start+i);

        if(shadow_area_dump_start+i == KASAN_LADDR_TO_SHADOW(addr)) {
            debug::out::printf("[%02x%c" , b , (shadow_size == 1) ? ']' : ' ');
        }
        else if(shadow_area_dump_start+i == KASAN_LADDR_TO_SHADOW(addr)+shadow_size-1 && shadow_size > 1) {
            debug::out::printf(" %02x]" , b);
        }
        else {
            debug::out::printf(" %02x " , b);
        }
    }
    
    // if(noabort) return;

    interrupt::controller::disable_all_interrupt();
    while(1) {
        ;
    }
}