#ifndef _ARCHITECTURE_LIMIT_HPP_
#define _ARCHITECTURE_LIMIT_HPP_

#define ARCHITECTURE_LIMIT 0xFFFFFFFFFFFFFFFF
#define ARCHITECTURE_LIMIT_BITS 64

#define INVALID ARCHITECTURE_LIMIT

// segmentation part
#define USE_SEGMENTATION
#define WARNING_NOT_USING_SEGMENTATION debug::out::printf(DEBUG_WARNING , "unusable function called : segmentation is disabled\n");
#define SEGMENT_MAXCOUNT          64

// interrupt part
#define USE_HARDWARE_INTERRUPT
#define WARNING_NOT_USING_HARDWARE_INTERRUPT debug::out::printf(DEBUG_WARNING , "unusable function called : interrupt is disabled\n");
#define GENERAL_INTERRUPT_MAXCOUNT     256

#endif