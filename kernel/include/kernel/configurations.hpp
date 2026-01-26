#ifndef _KERNEL_CONFIGURATIONS_HPP_
#define _KERNEL_CONFIGURATIONS_HPP_

#include <arch/configurations.hpp>

#ifndef yes
#define yes 1
#endif

#ifndef no
#define no 0
#endif

#ifndef WARNING
#define WARNING(x) debug::out::printf(DEBUG_WARNING , x);
#endif
#define CONFIG_KERNEL_HIGHERHALF yes
#define CONFIG_KERNEL_DEBUG_MAXLVL 10
#define CONFIG_KERNEL_STACK_SIZE 4194304
#define CONFIG_KERNEL_KSTRUCT_SIZE 524288

/****************** CONFIG_SEGMENTATION ******************/
#define CONFIG_USE_SEGMENTATION
#define CONFIG_SEGMENTATION_SEGMENT_MAXCOUNT 64
#define CONFIG_SEGMENTATION_TASKSEG_MODE yes
#define CONFIG_WARNING_NO_SEGMENTATION WARNING("Unused function called : segmentation is disabled\n");

/****************** CONFIG_INTERRUPT ******************/
#define CONFIG_USE_INTERRUPT
#define CONFIG_INTERRUPT_GENERAL_MAXCOUNT 256
#define CONFIG_WARNING_NO_INTERRUPT WARNING("Unused function called : interrupt is disabled\n");

/****************** CONFIG_KASAN ******************/
#define CONFIG_USE_KASAN
#define CONFIG_WARNING_NO_KASAN WARNING("Unused function called: KASan is disabled\n");

/****************** CONFIG_PAGING ******************/
#define CONFIG_USE_PAGING
#define CONFIG_WARNING_NO_PAGING WARNING("Unused function called : paging is disabled\n");

#endif