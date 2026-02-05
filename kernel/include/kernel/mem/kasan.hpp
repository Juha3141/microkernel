#ifndef _KERNEL_KASAN_HPP_
#define _KERNEL_KASAN_HPP_

#include <kernel/essentials.hpp>

#define KASAN_SHADOW_SHIFT 3
#define KASAN_GRANUL_SIZE  (1UL << KASAN_SHADOW_SHIFT)

#define KASAN_NULLPTR_PROTECTION 0x10000

#define KASAN_INTERNALS_INTERFACE extern "C"

namespace kasan {
    typedef max_t uptr;

    void init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end);
    bool is_enabled();
    void posion_address(max_t linear_address , max_t size);
    void unpoison_address(max_t linear_address , max_t size);

    void set_shadow_with_value(uptr addr , uptr size , byte value);
    
    bool check_address_validity(max_t linear_address , max_t size);
    void report_bug();
}

#endif