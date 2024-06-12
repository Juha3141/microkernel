#ifndef _ARCHITECTURE_LIMIT_HPP_
#define _ARCHITECTURE_LIMIT_HPP_

#define ARCHITECTURE_LIMIT 0xFFFFFFFFFFFFFFFF
#define ARCHITECTURE_LIMIT_BITS 64

#define INVALID ARCHITECTURE_LIMIT

// segmentation part
#define USE_SEGMENTATION
#define WARNING_NOT_USING_SEGMENTATION debug::out::printf(DEBUG_WARNING , "unusable function called : segmentation is disabled\n");
#define SEGMENT_MAXCOUNT               64

#define SEGMENTATION_TASKSEG_MODE      1

#endif