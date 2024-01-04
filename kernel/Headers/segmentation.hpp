#ifndef _SEGMENTATION_HPP_
#define _SEGMENTATION_HPP_

#include <interface_type.hpp>
#include <kmem_manager.hpp>

#define SEGMENT_MAXCOUNT          64

#define SEGMENT_TYPE_KERNEL_PRIVILEGE  0x01 
#define SEGMENT_TYPE_USER_PRIVILEGE    0x02
#define SEGMENT_TYPE_CODE_SEGMENT      0x04
#define SEGMENT_TYPE_DATA_SEGMENT      0x08
#define SEGMENT_TYPE_1TO1_CORRESPONDED 0x10
#define SEGMENT_TYPE_CUSTOM            0x00

// Default segments for system
#define SEGMENT_NAME_CODE      "code"
#define SEGMENT_NAME_DATA      "data"
#define SEGMENT_NAME_USER_CODE "user_code"
#define SEGMENT_NAME_USER_DATA "user_data"

#define SEGMENT_VALUE_INVALID  0xFFFF

typedef word segment_t;

namespace segmentation {
    struct SegmentsManager {
        void init(int entry_count);
        SINGLETON_PATTERN_KSTRUCT(struct SegmentsManager);
        
        void register_segment(const char *segment_name , segment_t segment_value);
        segment_t discard_segment(const char *segment_name);
        segment_t search_segment(const char *segment_name);

        int segments_maxcount;
        int segments_count;
        struct s_data_t {   
            bool occupied;
            char name[32];
            segment_t value;
        }*segments_data;
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
        segment_info user_code;
        segment_info user_data;
    };
    struct kernel_segments_value {
        segment_t kernel_code;
        segment_t kernel_data;
        segment_t user_code;
        segment_t user_data;        
    };
    void init(void);
    // register segment
    void register_segment(const char *segment_name , max_t start_address , max_t length , word segment_type);
    void discard_segment(const char *segment_name);
    
    segment_t get_segment_value(const char *segment_name);
    
    // renew the segment (if necessary)
    void set_to_code_segment(const char *segment_name , ptr_t new_point);
    void set_to_data_segment(const char *segment_name);
}

#endif