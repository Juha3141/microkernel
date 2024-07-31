#ifndef _CONFIG_CONFIGURATIONS_MACRO_
#define _CONFIG_CONFIGURATIONS_MACRO_

#define ARCHITECTURE_LIMIT 0xFFFFFFFFFFFFFFFF
#define ARCHITECTURE_LIMIT_BITS 64
#define INVALID ARCHITECTURE_LIMIT

/****************** CONFIG_SEGMENTATION ******************/
#define CONFIG_USE_SEGMENTATION
#define CONFIG_SEGMENTATION_SEGMENT_MAXCOUNT 64
#define CONFIG_SEGMENTATION_TASKSEG_MODE 1
#define CONFIG_WARNING_NO_SEGMENTATION WARNING("Unused function called : segmentation is disabled\n");

/****************** CONFIG_INTERRUPT ******************/
#define CONFIG_USE_INTERRUPT
#define CONFIG_INTERRUPT_GENERAL_MAXCOUNT 256
#define CONFIG_WARNING_NO_INTERRUPT WARNING("Unused function called : interrupt is disabled\n");

/****************** CONFIG_IST ******************/
#define CONFIG_USE_IST
#define CONFIG_IST_SIZE 2048

#endif