#include <kernel_info.hpp>
#include <debug.hpp>

void set_initial_kernel_info(void) {
    KernelInfo *kinfo = GLOBAL_KERNELINFO;
    memset(kinfo , 0 , sizeof(KernelInfo));
    
    /* Segmentation related */
    kinfo->conditions.task_segmentation_mode = KINFO_TASKSEG_LINEAR_CORRESPONDENCE;
    kinfo->conditions.use_ist = true;

    /* Multicore system related */
    kinfo->conditions.multicore_usage = Using;
    
    debug::out::printf("use_ist : %d\n" , kinfo->conditions.use_ist);
}