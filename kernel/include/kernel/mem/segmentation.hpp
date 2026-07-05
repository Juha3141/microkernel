#ifndef _SEGMENTATION_HPP_
#define _SEGMENTATION_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/kmem_manager.hpp>

#include <object_manager.hpp>

#define SEGMENT_PRIVILEGE_KERNEL       0x01
#define SEGMENT_PRIVILEGE_USER         0x02

#define SEGMENT_TYPE_CODE_SEGMENT      0x04
#define SEGMENT_TYPE_DATA_SEGMENT      0x08

#define SEGMENT_TYPE_SYSTEM_SEGMENT    0x10
#define SEGMENT_TYPE_TASK_SEGMENT      0x20

#define SEGMENT_TYPE_FLAT              0x40

#define SEGMENT_ID_KERNEL_CODE  0
#define SEGMENT_ID_KERNEL_DATA  1
#define SEGMENT_ID_FLAT_USER_CODE 2
#define SEGMENT_ID_FLAT_USER_DATA 3

#define SEGMENT_VALUE_INVALID  0xFFFF

typedef max_t segment_t;

namespace segmentation {
    // Packed data of information for segment
    struct segment_info_t {
        segment_t value;

        max_t start_address;
        max_t length;

        word segment_type;
        word privilege;
    };

    struct kernel_segments_info {
        segment_info_t kernel_code;
        segment_info_t kernel_data;
    };
    struct kernel_segments_value {
        segment_t kernel_code;
        segment_t kernel_data;    
    };
    void init();
    /// @brief Create a memory segment
    /// @param start_address physical start address of the segment
    /// @param length length
    /// @param segment_type type
    /// @param privilege privilege
    /// @return ID of the newly created segment
    max_t create_segment(max_t start_address , max_t length , word segment_type , word privilege);
    bool discard_segment(max_t id);
    
    /// @brief Get segment info
    /// @param id segment ID 
    /// @param segment_info Returned segment_info structure
    /// @return True if segment ID is valid, False if invalid
    bool get_segment_info(max_t id , segment_info_t &segment_info);
    bool get_segment_info_from_val(segment_t segment_value , segment_info_t &segment_info);
    /// @brief Get the segment value from ID
    /// @param id segment ID
    /// @return segment value
    segment_t get_segment_value(max_t id);
    
    /// @brief Set the system's code segment
    /// @param id ID of the segment that will be used
    /// @param new_point new point to jump right after changing the code segment
    void set_to_code_segment(max_t id , ptr_t new_point=INVALID);
    /// @brief Set the system's data segment
    /// @param id ID of the segment that will be used
    void set_to_data_segment(max_t id);
}

#endif