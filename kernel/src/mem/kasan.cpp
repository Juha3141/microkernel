#include <kernel/mem/kasan.hpp>
#include <string.hpp>
#include <kernel/interrupt/interrupt.hpp>

#define KASAN_SHADOW_MASK (1 << KASAN_SHADOW_SHIFT)-1

// Check out
// https://github.com/llvm-mirror/compiler-rt/blob/master/lib/asan/asan_interface_internal.h

/**
 * Microkernel utilizes physical memory by re-mapping them into one big contiguous memory block using the paging system.
 * Thus, Microkernel puts KASan's shadow memory at the end of the big memory chunk, in order to make memory layout more simple. 
 */

max_t kasan_shadow_memory_size     = 0x00;
// The linear memory address that Kasan will manage
max_t kasan_vma_start              = 0x00;
max_t kasan_vma_end                = 0x00;

#define READ false
#define WRITE true
#define CALLER_PC ((max_t)__builtin_return_address(0))

#define KASAN_LADDR_TO_SHADOW(addr)  \
    (((addr) >> KASAN_SHADOW_SHIFT)+CONFIG_KERNEL_KASAN_VMA)
#define KASAN_SHADOW_TO_LADDR(saddr) \
    ((((saddr)-kasan_shadow_memory_location) << 3))

__no_sanitize_address__
void kasan::init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end) {
    kasan_shadow_memory_size     = kasan_shadowmem_size;

    kasan_vma_start              = kernel_pool_start;
    kasan_vma_end                = kernel_pool_end;
}

#include <kernel/debug.hpp>

__no_sanitize_address__
void kasan::report_bug(max_t addr , max_t size , max_t buggy_shadow_address , byte is_write , max_t pc , bool noabort) {
    debug::out::printf(DEBUG_ERROR , "------------- kasan::report_bug() -------------\n");
    debug::out::printf(DEBUG_ERROR , "From trying to access 0x%llx sz=%d" , addr , size);
    if(buggy_shadow_address == 0x00) {
        debug::out::printf(DEBUG_ERROR , "    (Null Pointer Access)\n");
    }
    else {
        debug::out::printf(DEBUG_ERROR , "\nShadow addr : 0x%llx\n" , buggy_shadow_address);
    }
    debug::out::printf(DEBUG_ERROR , "RW : %c, pc=0x%llx\n" , is_write ? 'W' : 'R' , pc);
    debug::out::printf(DEBUG_ERROR , "-- Stack trace : \n");
    debug::out::printf(DEBUG_WARNING , "      3. pc=0x%llx\n" , __builtin_return_address(3));
    debug::out::printf(DEBUG_WARNING , "      4. pc=0x%llx\n" , __builtin_return_address(4));
    debug::out::printf(DEBUG_WARNING , "      5. pc=0x%llx\n" , __builtin_return_address(5));
    debug::out::printf(DEBUG_WARNING , "      6. pc=0x%llx\n" , __builtin_return_address(6));
    
    if(noabort) return;

    interrupt::controller::disable_all_interrupt();
    while(1) {
        ;
    }
}
__no_sanitize_address__
bool kasan::is_enabled() {
#ifdef CONFIG_USE_KASAN
    return (kasan_shadow_memory_size != 0x00);
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

struct kasan_global_info {
    // Starting address of the variable
    const void *start;
    // Variable size
    size_t size;
    // 32-bit aligned size of global including the redzone
    size_t size_with_redzone;
    // Symbol name
    const void *name;
    const void *module_name;
    unsigned long has_dynamic_init;
    void *location;
    unsigned int odr_indicator;
};

static void asan_register_global(kasan_global_info& global) {
  kasan::unpoison_address((max_t)global.start , global.size);

  size_t aligned_size = (global.size + KASAN_SHADOW_MASK) & ~KASAN_SHADOW_MASK;
  kasan::poison_address((unsigned long)global.start + aligned_size,
                global.size_with_redzone - aligned_size,
                KASAN_SHADOW_MAGIC_GLOBAL_REDZONE);
}

// 'globals' is an array of structures describing 'n' globals
KASAN_INTERNALS_INTERFACE void __asan_register_globals(struct kasan_global_info *globals , kasan::uptr n) {
    for(int i = 0; i < n; i++) { asan_register_global(globals[i]); }
}

KASAN_INTERNALS_INTERFACE void __asan_unregister_globals(struct kasan_global_info *globals) {

}

KASAN_INTERNALS_INTERFACE void __asan_before_dynamic_init() {
    
}

KASAN_INTERNALS_INTERFACE void __asan_after_dynamic_init() {
    
}

// We don't have to care about handling no return function, because we have -Werror=return-type
KASAN_INTERNALS_INTERFACE void __asan_handle_no_return() {}

// FIXME : I don't really work as intended. 
//         Ian, you need to really understand how KASan works and really implement these functions all by yourself.
__no_sanitize_address__
void kasan::poison_address(max_t linear_address , max_t size , byte value) {
    if(!kasan::is_enabled()) return;

    max_t shadow_start = KASAN_LADDR_TO_SHADOW(linear_address);
    max_t shadow_end   = KASAN_LADDR_TO_SHADOW(linear_address+size-1)+1;
    max_t shadow_len = shadow_end-shadow_start;
    debug::out::printf(DEBUG_INFO , "Poisoning shadow: 0x%llx~0x%llx (SHW:0x%llx~0x%llx) = %02x, shadow len=%d\n" , linear_address , linear_address+size , shadow_start , shadow_end , value , shadow_len);

    memset((void *)shadow_start , value , shadow_len);
}

__no_sanitize_address__
void kasan::unpoison_address(max_t linear_address , max_t size) {
    if(!kasan::is_enabled()) return;
    
    // "round down" the size to the grain size
    poison_address(linear_address , size & (~KASAN_SHADOW_MASK) , KASAN_SHADOW_MAGIC_UNPOISONED);
    
    // if the size is unaligned to the size, 
    if(size & KASAN_SHADOW_MASK) {
        byte *shadow = (byte *)KASAN_LADDR_TO_SHADOW(linear_address+size);
        *shadow = size & KASAN_SHADOW_MASK;
    }
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
bool kasan::check_address_validity(max_t linear_address , max_t size , byte is_write , max_t pc , bool noabort) {
    if(!kasan::is_enabled()) return true;
    if(size == 0)     return true;
    if(linear_address < KASAN_NULLPTR_PROTECTION) kasan::report_bug(linear_address , size , 0x00 , is_write , pc , noabort);
    
    max_t buggy_shadow_addr = get_poisoned_shadow_address(linear_address , size);
    if(buggy_shadow_addr != 0) kasan::report_bug(linear_address , size , buggy_shadow_addr , is_write , pc , noabort);

    return true;
}

KASAN_INTERNALS_INTERFACE void __asan_poison_stack_memory(kasan::uptr addr , kasan::uptr size) {
    kasan::poison_address((max_t)addr , size , KASAN_SHADOW_MAGIC_STACK_AFTER_RETURN);
}

KASAN_INTERNALS_INTERFACE void __asan_unpoison_stack_memory(kasan::uptr addr , kasan::uptr size) {
    kasan::unpoison_address((max_t)addr , size);
}

KASAN_INTERNALS_INTERFACE void __asan_poison_memory_region(void const volatile *addr , kasan::uptr size) {
    kasan::poison_address((max_t)addr , size , KASAN_SHADOW_MAGIC_RESERVED);
}

KASAN_INTERNALS_INTERFACE void __asan_unpoison_memory_region(void const volatile *addr , kasan::uptr size) {
    kasan::poison_address((max_t)addr , size , KASAN_SHADOW_MAGIC_UNPOISONED);
}

KASAN_INTERNALS_INTERFACE void __asan_alloca_poison(void *addr , kasan::uptr size) {
    debug::out::printf("Warning : You're seeing this message because you're using variable string at pc=0x%llx\n" , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_allocas_unpoison(void *stack_top , kasan::uptr stack_bottom) {
    
}

KASAN_INTERNALS_INTERFACE void __asan_report_error(kasan::uptr pc , kasan::uptr bp , kasan::uptr sp , kasan::uptr addr , int is_write , kasan::uptr access_size , uint32_t exp) {
    kasan::report_bug(addr , access_size , get_poisoned_shadow_address(addr , access_size) , is_write , pc);
}

KASAN_INTERNALS_INTERFACE void __asan_report_load_n(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , READ , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_report_store_n(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , WRITE , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_report_load_n_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , READ , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_report_store_n_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , WRITE , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_loadN(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , READ , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_storeN(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , WRITE , CALLER_PC);
}

/*** noabort : Do not abort the kernel, continue excution even after the memory bug ***/
KASAN_INTERNALS_INTERFACE void __asan_loadN_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , READ , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_storeN_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , WRITE , CALLER_PC , true);
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
    memset((void *)addr , 0x##value , size); }

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

#define DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(N) \
KASAN_INTERNALS_INTERFACE void __asan_report_load##N##_noabort(kasan::uptr p) { __asan_report_load_n_noabort(p , N); } \
KASAN_INTERNALS_INTERFACE void __asan_report_store##N##_noabort(kasan::uptr p) { __asan_report_store_n_noabort(p , N); }

DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(1)
DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(2)
DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(4)
DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(8)
DECLARE_ASAN_REPORT_LOAD_STORE_NOABORT(16)

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