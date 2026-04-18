#include <kernel/mem/kasan.hpp>
#include <string.hpp>

// Some codes are referenced(copied...) from Linux Kernel!

// Check out
// https://github.com/llvm-mirror/compiler-rt/blob/master/lib/asan/asan_interface_internal.h

max_t kasan_shadow_memory_size     = 0x00;
// The linear memory address that Kasan will manage
max_t kasan_vma_start              = 0x00;
max_t kasan_vma_end                = 0x00;

__no_sanitize_address__
void kasan::init(max_t kasan_shadowmem_size , max_t kernel_pool_start , max_t kernel_pool_end) {
    kasan_shadow_memory_size     = kasan_shadowmem_size;

    kasan_vma_start              = kernel_pool_start;
    kasan_vma_end                = kernel_pool_end;
}

#include <kernel/debug.hpp>

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

KASAN_INTERNALS_INTERFACE void __asan_unregister_globals(struct kasan_global_info *globals) {}
KASAN_INTERNALS_INTERFACE void __asan_before_dynamic_init() {}
KASAN_INTERNALS_INTERFACE void __asan_after_dynamic_init() {}

// We don't have to care about handling no return function, because we have -Werror=return-type
KASAN_INTERNALS_INTERFACE void __asan_handle_no_return() {}

__no_sanitize_address__
/// @brief Note: linear address should always be aligned to shadow size!
/// @param laddr 8-byte aligned address
/// @param size 
/// @param value 
void kasan::poison_address(max_t laddr , max_t size , byte value) {
    if(!kasan::is_enabled()) return;
    if(size < KASAN_GRANUL_SIZE) {
        debug::out::printf(DEBUG_WARNING , "Unsupported poisoning size : %lld\n" , size);
        debug::out::printf(DEBUG_WARNING , "Caller = 0x%llx\n" , CALLER_PC);
        return;
    } 

    if(!is_aligned(laddr , KASAN_GRANUL_SIZE)) {
        *((byte *)KASAN_LADDR_TO_SHADOW(laddr)) = value == KASAN_SHADOW_MAGIC_HEAP_FREE ? value : (laddr & KASAN_SHADOW_MASK);
        laddr = align_round_up(laddr , KASAN_GRANUL_SIZE);
        size--;
    }
    
    byte last_shadow_byte = size & KASAN_SHADOW_MASK;
    max_t shadow_start = KASAN_LADDR_TO_SHADOW(laddr);
    max_t shadow_end   = KASAN_LADDR_TO_SHADOW(align_round_down(laddr+size , KASAN_GRANUL_SIZE)); // inclusive

    for(max_t a = shadow_start; a < shadow_end; a++) {
        *((byte *)a) = value;
    }
}

__no_sanitize_address__
void kasan::unpoison_address(max_t linear_address , max_t size) {
    if(!kasan::is_enabled()) return;
    poison_address(linear_address , align_round_up(size , KASAN_GRANUL_SIZE) , KASAN_SHADOW_MAGIC_UNPOISONED);
}

__no_sanitize_address__
bool kasan::is_poisoned_1(max_t addr) {
    if(!kasan::is_enabled()) return false;
    if(addr < KASAN_NULLPTR_PROTECTION) return true;
    
    char shadow_val = *(char *)KASAN_LADDR_TO_SHADOW(addr);

    if(shadow_val) {
        /* Shadow value indicates the number of bytes accessible from the start in the given 8-byte chunk
         * last_accessible_byte : the offset from the 8-byte aligned chunk (addr & SHADOW_MASK)
         * 
         * Say last_accessible_byte is 5. If shadow_val is 5, then, only the first five bytes are accessible, which is
         * byte 0,1,2,3 and 4.
         * 
         *                last_accessible_byte
         *                       V 
         * +---+---+---+---+---+---+---+---+
         * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
         * +---+---+---+---+---+---+---+---+
         * <===================>
         *    Accessible when
         *    shadow val = 5
         * 
         * This is a faulty memory access, from looking at the layout. So, 
         * 
         * If last_accessible_byte >= shadow_val, then the memory that's being accessed is poisoned. 
         */

        char last_accessible_byte = (addr & KASAN_SHADOW_MASK);
        return last_accessible_byte >= shadow_val;
    }

    return false;
}

__no_sanitize_address__
bool kasan::is_poisoned_2_4_8(max_t addr , size_t size) {
    unsigned char *shadow_addr = (unsigned char *)KASAN_LADDR_TO_SHADOW(addr);
    
    /* Check if the memory access crosses 8(shadow-size)-byte boundary. 
     * If so, map into 2 shadow bytes.
     */
    /* 2/4/8 memory access across two shadow bytes
     *
     *                               trying to access
     *                     <=================================>
     * +---+---+---+---+---+---+---+---+ +---+---+---+---+---+---+---+---+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
     * +---+---+---+---+---+---+---+---+ +---+---+---+---+---+---+---+---+ 
     *             shadow_addr       ^
     *                              rear
     * 
     * KASan always indicates the accessibility from the start of the memory chunk
     * So, if shadow_addr is nonzero, that means the very rear byte of the memory (7th) is always poisoned, and
     * since the memory access is overlapping this memory region, the access is poisoned.
     * 
     * Checking procedure for the other chunk is the same as checking 2/4/8 memory access within the one KASan shadow.
     */
    if(((addr+size-1) & KASAN_SHADOW_MASK) < size-1) {
        return *shadow_addr||is_poisoned_1(addr+size-1);
    }

    /* 2/4/8 memory access is within the one KASan shadow memory : 
     * 
     *          trying to access 
     *         <===============>
     * +---+---+---+---+---+---+---+---+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | shadow_val = 4
     * +---+---+---+---+---+---+---+---+
     * <=======================>
     *         accessible
     * 
     * KASan always indicates the accessibility from the start of the memory chunk(IMPORTANT SO I ITERATE!)
     * If the last byte we're trying to access(addr+size-1) is accessible, 
     * that means that the address less than so is all accessible within the memory chunk
     * 
     * So we only have to check the last byte of the memory!
     */
    return is_poisoned_1(addr+size-1);
}

__no_sanitize_address__
bool kasan::is_poisoned_16(max_t addr) {
    unsigned short *shadow_addr = (unsigned short *)KASAN_LADDR_TO_SHADOW(addr);

    // if it's not aligned to 16 bytes, the memory access spans three memory chunks
	if(!is_aligned(addr , KASAN_GRANUL_SIZE)) {
        /* Check first two bytes by just checking if shadow values are nonzero,
         * Check the last byte with is_poisoned_1 
         * This will make sense now that you understood how KASan works! */
        return *shadow_addr||is_poisoned_1(addr + 15);
    }
    // Just return whether it's zero or nonzero.
    // In 16-bit aligned case, the memory access is valid if and only if the shadow bytes are all zero. 
    return *shadow_addr;
}

__no_sanitize_address__
bool kasan::is_poisoned_N(max_t addr , size_t size) {
    max_t shadow_start = KASAN_LADDR_TO_SHADOW(addr);
    max_t shadow_end   = KASAN_LADDR_TO_SHADOW(addr+size-1)+1;

    max_t poisoned_addr = 0;
    for(max_t sa = shadow_start; sa < shadow_end; sa++) {
        if(*((byte *)sa) != KASAN_SHADOW_MAGIC_UNPOISONED) {
            poisoned_addr = sa;
            break;
        }
    }

    if(poisoned_addr) {
        max_t last_byte_addr = addr+size-1;
        char *last_shadow_byte = (char *)KASAN_LADDR_TO_SHADOW(last_byte_addr);
        char last_accessible_byte = last_byte_addr & KASAN_SHADOW_MASK;

        if(poisoned_addr != (max_t)last_shadow_byte||last_accessible_byte >= *last_shadow_byte) {
            return true;
        }
    }
    return false;
}

__no_sanitize_address__
bool kasan::check_address_validity(max_t linear_address , max_t size , byte is_write , max_t pc , bool noabort) {
    if(!kasan::is_enabled()) return true;
    if(size == 0)     return true;
    if(linear_address < KASAN_NULLPTR_PROTECTION) kasan::report_bug(linear_address , size , is_write , pc , noabort);
    
    bool is_poisoned = false;
    switch(size) {
        case 1:
            is_poisoned = is_poisoned_1(linear_address);
            break;
        case 2:
        case 4:
        case 8:
            is_poisoned = is_poisoned_2_4_8(linear_address , size);
            break;
        case 16:
            is_poisoned = is_poisoned_16(linear_address);
            break;
        default:
            is_poisoned = is_poisoned_N(linear_address , size);
            break;
    }

    if(is_poisoned) kasan::report_bug(linear_address , size , is_write , pc , noabort);

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
    debug::out::printf(DEBUG_WARNING , "Warning : You're seeing this message because you're using variable string at pc=0x%llx\n" , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_allocas_unpoison(void *stack_top , kasan::uptr stack_bottom) {
    
}

KASAN_INTERNALS_INTERFACE void __asan_report_error(kasan::uptr pc , kasan::uptr bp , kasan::uptr sp , kasan::uptr addr , int is_write , kasan::uptr access_size , uint32_t exp) {
    kasan::report_bug(addr , access_size , is_write , pc);
}

KASAN_INTERNALS_INTERFACE void __asan_report_load_n(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_READ , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_report_store_n(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_WRITE , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_report_load_n_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_READ , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_report_store_n_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_WRITE , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_loadN(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_READ , CALLER_PC);
}

KASAN_INTERNALS_INTERFACE void __asan_storeN(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_WRITE , CALLER_PC);
}

/*** noabort : Do not abort the kernel, continue excution even after the memory bug ***/
KASAN_INTERNALS_INTERFACE void __asan_loadN_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_READ , CALLER_PC , true);
}

KASAN_INTERNALS_INTERFACE void __asan_storeN_noabort(kasan::uptr p , kasan::uptr size) {
    kasan::check_address_validity(p , size , KASAN_WRITE , CALLER_PC , true);
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
    kasan::poison_address(addr , size , 0x##value); }

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