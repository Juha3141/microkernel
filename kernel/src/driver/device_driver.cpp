#include <kernel/driver/device_driver.hpp>

#define CALL_FPTR_FROM_SECTION(section_start , section_end) for(qword func_ptr_ptr = (qword)&section_start; func_ptr_ptr < (qword)&section_end; func_ptr_ptr += sizeof(driver_init_func_ptr_t)) { \
        qword func_ptr = *((qword *)func_ptr_ptr); \
        ((void(*)(void))func_ptr)(); \
    }

extern qword __drivers_init_start__;
extern qword __drivers_init_end__;

extern qword __fs_drivers_init_start__;
extern qword __fs_drivers_init_end__;

void register_file_system_drivers(void) { CALL_FPTR_FROM_SECTION(__fs_drivers_init_start__ , __fs_drivers_init_end__); }
void register_kernel_drivers(void)      { CALL_FPTR_FROM_SECTION(__drivers_init_start__ , __drivers_init_end__); }