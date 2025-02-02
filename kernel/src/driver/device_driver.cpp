#include <kernel/driver/device_driver.hpp>

extern qword __drivers_init_start__;
extern qword __drivers_init_end__;

void register_kernel_drivers(void) {
    for(qword func_ptr_ptr = (qword)&__drivers_init_start__; func_ptr_ptr < (qword)&__drivers_init_end__; func_ptr_ptr += sizeof(driver_init_func_ptr_t)) {
        qword func_ptr = *((qword *)func_ptr_ptr);
        ((void(*)(void))func_ptr)();
    }
}