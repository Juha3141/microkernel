#ifndef _CONFIG_CONFIG_
#define _CONFIG_CONFIG_


/****************** CONFIG_INTERRUPT ******************/
#define CONFIG_USE_INTERRUPT
#define CONFIG_GENERAL_INTERRUPT_MAXCOUNT 256
#define CONFIG_WARNING_NO_ WARNING("unusable function called : interrupt is disabled\n");

/****************** CONFIG_IST ******************/
#define CONFIG_USE_IST
#define CONFIG_IST_STACK_SIZE 2048

/****************** CONFIG_SEGMENTATION ******************/
#define CONFIG_USE_SEGMENTATION
#define CONFIG_SEGMENTATION_MAXCOUNT 64
#define CONFIG_WARNING_NO_ WARNING("unusable function called : segmentation is disabled\n");
#define CONFIG_HELLO_WORLD "Hello, world!"

#endif