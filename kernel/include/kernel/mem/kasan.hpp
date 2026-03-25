#ifndef _KERNEL_KASAN_HPP_
#define _KERNEL_KASAN_HPP_

#include <kernel/essentials.hpp>

#define KASAN_SHADOW_SHIFT 3
#define KASAN_GRANUL_SIZE  (1UL << KASAN_SHADOW_SHIFT)

#define KASAN_NULLPTR_PROTECTION 0x500

#define KASAN_INTERNALS_INTERFACE extern "C"

#define KASAN_SHADOW_MAGIC_UNPOISONED        0x00
#define KASAN_SHADOW_MAGIC_RESERVED          0xff
#define KASAN_SHADOW_MAGIC_GLOBAL_REDZONE    0xf9
#define KASAN_SHADOW_MAGIC_HEAP_HEAD_REDZONE 0xfa
#define KASAN_SHADOW_MAGIC_HEAP_TAIL_REDZONE 0xfb
#define KASAN_SHADOW_MAGIC_HEAP_FREE         0xfd
#define KASAN_HEAP_HEAD_REDZONE_SIZE 16
#define KASAN_HEAP_TAIL_REDZONE_SIZE 10

namespace kasan {
    typedef max_t uptr;

    void init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end);
    bool is_enabled();
    void poison_address(max_t linear_address , max_t size , byte value);
    void unpoison_address(max_t linear_address , max_t size);
    
    bool check_address_validity(max_t linear_address , max_t size);
    void report_bug();
}

#endif