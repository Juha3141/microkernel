#ifndef _KERNEL_KASAN_HPP_
#define _KERNEL_KASAN_HPP_

#include <kernel/essentials.hpp>

#define KASAN_SHADOW_SHIFT 3

namespace kasan {
    void init(max_t shadowmem_location , max_t shadowmem_size , max_t physmem_start);
    void posion_address(max_t phys_address , max_t size);
    void unpoison_address(max_t phys_address , max_t size);
    
    bool check_address_validity(max_t phys_address , max_t size);
}

#endif