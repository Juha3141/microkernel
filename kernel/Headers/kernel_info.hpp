/**
 * @file kernel_info.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Defined structure for universal kernel information
 * @date 2024-01-01
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */

#ifndef _KERNEL_INFO_HPP_
#define _KERNEL_INFO_HPP_

#include <interface_type.hpp>
#include <kmem_manager.hpp>

#define KINFO_SEGMENTATION_MODE_LINEAR_CORRESPONDENCE 1
#define KINFO_SEGMENTATION_MODE_SEGMENTED             2

enum state_t {
    Using=0 , NotUsing=1 , Disabled=2
};

/* KernelInfo structure : Contains the critical informations about kernel.
 * The values are preset by initialization function, and works according to values
 * Say : if you don't want OS to be multi-core, just disable multicore
 * 
 * If value is "Disabled," the system will be disabled permanently.
 * This is different to "Using" and "Not Using" value, which is dependently determined by
 * device driver or other kernel system(or mostly HAL)s
 */

struct KernelInfo {
    // Use this structure as global variables
    SINGLETON_PATTERN_KSTRUCT(struct KernelInfo);
    union {
        /* Segmentation related */
        word segmentation_mode; // segmentation mode
        bool use_ist;           // interrupt stack table
    
        /* Multicore system related */
        state_t multicore_usage;
        
    }predet;
    
    // Determined by other parts of system
    struct ist_info_t {
        size_t ist_count; // number of ist
        max_t ist_size;  // size of ist
        max_t ist_ptr;   // ist starting pointer
    }ist_info;

    struct multicore_info_t {
        size_t core_count;
        max_t multicore_entrypoint;
    }multicore_info;
    // add more
};

void set_initial_kernel_info(void);

#endif