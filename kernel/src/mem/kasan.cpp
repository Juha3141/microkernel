#include <kernel/mem/kasan.hpp>
#include <string.hpp>

// Check out
// https://github.com/llvm-mirror/compiler-rt/blob/master/lib/asan/asan_interface_internal.h

/**
 * Microkernel utilizes physical memory by re-mapping them into one big contiguous memory block using the paging system.
 * Thus, Microkernel puts KASan's shadow memory at the end of the big memory chunk, in order to make memory layout more simple. 
 */

max_t kasan_shadow_memory_location = 0x00;
max_t kasan_shadow_memory_size     = 0x00;
// The linear memory address that Kasan will manage
max_t kasan_vma_start              = 0x00;
max_t kasan_vma_end                = 0x00;

#define KASAN_LADDR_TO_SHADOW(addr)  \
    ((((addr)-kasan_vma_start) >> KASAN_SHADOW_SHIFT)+kasan_shadow_memory_location)
#define KASAN_SHADOW_TO_LADDR(saddr) \
    ((((saddr)-kasan_shadow_memory_location) << 3)+kasan_vma_start)

__no_sanitize_address__ void kasan::init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end) {
    kasan_shadow_memory_size     = kasan_shadowmem_size;
    kasan_shadow_memory_location = CONFIG_KERNEL_KASAN_VMA;

    kasan_vma_start              = kernel_pool_start;
    kasan_vma_end                = kernel_pool_end;
    
    // debug::out::printf("kasan_shadow_memory_location : 0x%llx\n" , kasan_shadow_memory_location);
    // debug::out::printf("kasan_shadow_memory_size     : 0x%llx\n" , kasan_shadow_memory_size);
    // debug::out::printf("(Kasan will be covering memory from 0x%llx to 0x%llx)\n" , kasan_vma_start , linearly_mapped_free_pool_end);
}

bool kasan::is_enabled() {
#ifdef CONFIG_USE_KASAN
    return (kasan_shadow_memory_location != 0x00);
#else
    return false;
#endif
}

// This structure is used to describe the source location of a place where
// global was defined.
struct kasan_global_source_location {
    const char *filename;
    int line_number;
    int column_number;
};

// This structure describes an instrumented global variable.
struct kasan_global_vars {
    kasan::uptr beg; // The address of the global
    kasan::uptr size; // The original size of the global
    kasan::uptr size_with_redzone; // The size with the redzone
    const char *name;
    const char *module_name;

    kasan::uptr has_dynamic_init;
    kasan_global_source_location *location;
    kasan::uptr odr_indicator;
};

// 'globals' is an array of structures describing 'n' globals
KASAN_INTERNALS_INTERFACE void __asan_register_globals(struct kasan_global_info *globals , kasan::uptr n) {

}

KASAN_INTERNALS_INTERFACE void __asan_unregister_globals(struct kasan_global_info *globals) {

}

KASAN_INTERNALS_INTERFACE void __asan_before_dynamic_init() {
    
}

KASAN_INTERNALS_INTERFACE void __asan_after_dynamic_init() {
    
}

// We don't have to care about handling no return function, because we have -Werror=return-type
KASAN_INTERNALS_INTERFACE void __asan_handle_no_return() {}

void kasan::posion_address(max_t linear_address , max_t size) {

}

void kasan::unpoison_address(max_t linear_address , max_t size) {

}

// Sets of bytes of the given range of the shadow memory into specific value
void kasan::set_shadow_with_value(kasan::uptr addr , kasan::uptr size , byte value) {

}

bool kasan::check_address_validity(max_t linear_address , max_t size) {
    if(!is_enabled()) return true;
    if(size == 0)     return true;
    // FIXME : I might cause an issue in the future
    if(linear_address > KASAN_NULLPTR_PROTECTION && linear_address < kasan_vma_start) return true;

    return true;
}

KASAN_INTERNALS_INTERFACE void __asan_poison_stack_memory(kasan::uptr addr , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_unpoison_stack_memory(kasan::uptr addr , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_poison_memory_region(void const volatile *addr , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_unpoison_memory_region(void const volatile *addr , kasan::uptr size) {
    
}

KASAN_INTERNALS_INTERFACE int __asan_address_is_poisoned(void const volatile *addr) {
    return 0;
}

KASAN_INTERNALS_INTERFACE kasan::uptr __asan_region_is_poisoned(kasan::uptr beg , kasan::uptr size) {
    return 0;
}

KASAN_INTERNALS_INTERFACE void __asan_report_error(kasan::uptr ptr , kasan::uptr bp , kasan::uptr sp , kasan::uptr addr , int is_write , kasan::uptr access_size , uint32_t exp) {
    
}

KASAN_INTERNALS_INTERFACE void __asan_report_load_n(kasan::uptr p , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_report_store_n(kasan::uptr p , kasan::uptr size) {
    
}

KASAN_INTERNALS_INTERFACE void __asan_loadN(kasan::uptr p , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_storeN(kasan::uptr p , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_loadN_noabort(kasan::uptr p , kasan::uptr size) {

}

KASAN_INTERNALS_INTERFACE void __asan_storeN_noabort(kasan::uptr p , kasan::uptr size) {

}

// Memcpy/Memset/Memmove with memory sanitization
KASAN_INTERNALS_INTERFACE void *__asan_memcpy(void *dst , const void *src , kasan::uptr size) {
    return memcpy(dst , src , size);
}

KASAN_INTERNALS_INTERFACE void *__asan_memset(void *s , int c , kasan::uptr n) {
    return memset(s , c , n);
}

KASAN_INTERNALS_INTERFACE void *__asan_memmove(void *dest , const void * src , kasan::uptr n) {
    return memmove(dest , src , n);
}

#define DECLARE_ASAN_SET_SHADOW(value) \
KASAN_INTERNALS_INTERFACE void __asan_set_shadow_##value(kasan::uptr addr , kasan::uptr size) { \
    kasan::set_shadow_with_value(addr , size , 0x##value); }

DECLARE_ASAN_SET_SHADOW(00)
DECLARE_ASAN_SET_SHADOW(f1)
DECLARE_ASAN_SET_SHADOW(f2)
DECLARE_ASAN_SET_SHADOW(f3)
DECLARE_ASAN_SET_SHADOW(f5)
DECLARE_ASAN_SET_SHADOW(f8)

#define DECLARE_ASAN_REPORT_LOAD_STORE(N) \
KASAN_INTERNALS_INTERFACE void __asan_report_load##N(kasan::uptr p) { __asan_report_load_n(p , N); } \
KASAN_INTERNALS_INTERFACE void __asan_report_store##N(kasan::uptr p) { __asan_report_store_n(p , N); }

DECLARE_ASAN_REPORT_LOAD_STORE(1)
DECLARE_ASAN_REPORT_LOAD_STORE(2)
DECLARE_ASAN_REPORT_LOAD_STORE(4)
DECLARE_ASAN_REPORT_LOAD_STORE(8)
DECLARE_ASAN_REPORT_LOAD_STORE(16)

#define DECLARE_ASAN_LOAD_STORE(N) \
KASAN_INTERNALS_INTERFACE void __asan_load##N(kasan::uptr p) { __asan_loadN(p , N); } \
KASAN_INTERNALS_INTERFACE void __asan_store##N(kasan::uptr p) { __asan_storeN(p , N); }

DECLARE_ASAN_LOAD_STORE(1)
DECLARE_ASAN_LOAD_STORE(2)
DECLARE_ASAN_LOAD_STORE(4)
DECLARE_ASAN_LOAD_STORE(8)
DECLARE_ASAN_LOAD_STORE(16)

#define DECLARE_ASAN_LOAD_STORE_NOABORT(N) \
KASAN_INTERNALS_INTERFACE void __asan_load##N##_noabort(kasan::uptr p) { __asan_loadN_noabort(p , N); }\
KASAN_INTERNALS_INTERFACE void __asan_store##N##_noabort(kasan::uptr p) { __asan_storeN_noabort(p , N); }

DECLARE_ASAN_LOAD_STORE_NOABORT(1)
DECLARE_ASAN_LOAD_STORE_NOABORT(2)
DECLARE_ASAN_LOAD_STORE_NOABORT(4)
DECLARE_ASAN_LOAD_STORE_NOABORT(8)
DECLARE_ASAN_LOAD_STORE_NOABORT(16)