#include <kernel_info.hpp>
#include <debug.hpp>

void set_initial_kernel_info(void) {
    KernelInfo *kinfo = KernelInfo::get_self();
    memset(kinfo , 0 , sizeof(KernelInfo));
    
    /* Segmentation related */
    kinfo->predet.segmentation_mode = KINFO_SEGMENTATION_MODE_LINEAR_CORRESPONDENCE;
    kinfo->predet.use_ist = true;

    /* Multicore system related */
    kinfo->predet.multicore_usage = Using;
    
}