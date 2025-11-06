#include <kernel/mem/kasan.hpp>

typedef max_t sanitizer_uptr;

max_t kasan_shadow_memory_location;
max_t kasan_shadow_memory_size;
max_t kasan_physical_memory_start;

void kasan::init(max_t shadowmem_location , max_t shadowmem_size , max_t physmem_start) {
    
}

void __asan_load1(sanitizer_uptr p) {

}
