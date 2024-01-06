#ifndef _SEGMENTATION_HPP_
#define _SEGMENTATION_HPP_

#include <interface_type.hpp>
#include <kmem_manager.hpp>

#define SEGMENT_MAXCOUNT          64

#define SEGMENT_TYPE_KERNEL_PRIVILEGE  0x01
#define SEGMENT_TYPE_USER_PRIVILEGE    0x02
#define SEGMENT_TYPE_CODE_SEGMENT      0x04
#define SEGMENT_TYPE_DATA_SEGMENT      0x08
#define SEGMENT_TYPE_SYSTEM_SEGMENT    0x10
#define SEGMENT_TYPE_TASK_SEGMENT      0x20

// Default segments for system
#define SEGMENT_NAME_CODE              "kernel_code"
#define SEGMENT_NAME_DATA              "kernel_data"
#define SEGMENT_NAME_1TO1_USER_CODE    "general_user_code"
#define SEGMENT_NAME_1TO1_USER_DATA    "general_user_data"

#define SEGMENT_VALUE_INVALID  0xFFFF

typedef word segment_t;

namespace segmentation {
    struct segment_info_t {
        bool occupied;
        char name[32];
        segment_t value;
        word segment_type;

        max_t task_id;
    };
    struct SegmentsManager {
        void init(int entry_count);
        SINGLETON_PATTERN_KSTRUCT(struct SegmentsManager);
        
        bool register_segment(const char *segment_name , segment_t segment_value , word segment_type , max_t task_id=INVALID);
        segment_t discard_segment(const char *segment_name);
        segment_t search_segment(const char *segment_name);

        segment_info_t *segments_data;
        int segments_maxcount;
        int segments_count;
    };
    // Packed data of information for segment
    struct segment_info {
        max_t start_address;
        max_t length;
        word segment_type;
    };
    struct kernel_segments_info {
        segment_info kernel_code;
        segment_info kernel_data;
    };
    struct kernel_segments_value {
        segment_t kernel_code;
        segment_t kernel_data;    
    };
    void init(void);
    // register segment
    bool register_segment(const char *segment_name , max_t start_address , max_t length , word segment_type , max_t task_id=INVALID);
    bool discard_segment(const char *segment_name);
    
    bool get_segment_info(const char *segment_name , segment_info_t &segment_info);
    bool get_segment_info(segment_t segment_value , segment_info_t &segment_info);
    segment_t get_segment_value(const char *segment_name);
    
    // renew the segment (if necessary)
    void set_to_code_segment(const char *segment_name , ptr_t new_point=INVALID);
    void set_to_data_segment(const char *segment_name);
}

#endif