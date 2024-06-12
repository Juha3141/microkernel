// should be auto-generated from the configuration file

#ifndef _CONFIG_INTERRUPT_HPP_
#define _CONFIG_INTERRUPT_HPP_

#define WARNING(str) debug::out::printf(DEBUG_WARNING , str)

/******************* CONFIG_USE_HARDWARE_INTERRUPT *******************/
#define CONFIG_USE_HARDWARE_INTERRUPT
#define CONFIG_WARNING_USE_HARDWARE_INTERRUPT WARNING("unusable function called : interrupt is disabled\n");

#define CONFIG_GENERAL_INTERRUPT_MAXCOUNT     256

/******************* CONFIG_USE_IST *******************/
#define CONFIG_USE_IST
#define CONFIG_IST_SIZE           2048



#endif
