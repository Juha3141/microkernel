#ifndef _KERNEL_KASAN_HPP_
#define _KERNEL_KASAN_HPP_

#include <kernel/essentials.hpp>

#define KASAN_SHADOW_SHIFT 3
#define KASAN_GRANUL_SIZE  (1UL << KASAN_SHADOW_SHIFT)

#define KASAN_NULLPTR_PROTECTION 0x500

#define KASAN_INTERNALS_INTERFACE __no_sanitize_address__ extern "C"

#define KASAN_SHADOW_MAGIC_UNPOISONED          0x00
#define KASAN_SHADOW_MAGIC_RESERVED            0xff
#define KASAN_SHADOW_MAGIC_GLOBAL_REDZONE      0xf9
#define KASAN_SHADOW_MAGIC_HEAP_HEAD_REDZONE   0xfa
#define KASAN_SHADOW_MAGIC_HEAP_TAIL_REDZONE   0xfb
#define KASAN_SHADOW_MAGIC_HEAP_FREE           0xfd
#define KASAN_SHADOW_MAGIC_STACK_AFTER_RETURN  0xf5
#define KASAN_SHADOW_MAGIC_STACK_AFTER_SCOPE   0xf8
#define KASAN_HEAP_HEAD_REDZONE_SIZE 8
#define KASAN_HEAP_TAIL_REDZONE_SIZE 8

#define KASAN_READ false
#define KASAN_WRITE true
#define CALLER_PC ((max_t)__builtin_return_address(0))

namespace kasan {
    typedef max_t uptr;

    void init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end);
    bool is_enabled();
    void poison_address(max_t linear_address , max_t size , byte value);
    void unpoison_address(max_t linear_address , max_t size);
    
    bool check_address_validity(max_t linear_address , max_t size , byte is_write , max_t pc , bool noabort=false);
    /// @brief KASan bug report function
    /// @param addr 
    /// @param size 
    /// @param buggy_shadow_address 
    /// @param is_write 
    /// @param pc Program Counter
    void report_bug(max_t addr , max_t size , max_t buggy_shadow_address , byte is_write , max_t pc , bool noabort=false);
}

#endif