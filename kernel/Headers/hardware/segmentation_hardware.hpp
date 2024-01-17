#ifndef _SEGMENTATION_HARDWARE_HPP_
#define _SEGMENTATION_HARDWARE_HPP_

#include <segmentation.hpp>
#include <kernel_info.hpp>

namespace segmentation {
    namespace hardware {
        void set_essential_kernel_segment(kernel_segments_info &kseginfo);
        
        void init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues);
        void init_ist(KernelInfo::ist_info_t &ist_info); // If necessary

        segment_t register_system_segment(max_t start_address , max_t length , word segment_type);
        segment_t register_task_segment(max_t start_address , max_t length , word segment_type);
        void discard_segment(segment_t segment);

        void set_to_code_segment(segment_t segment);
        void set_to_code_segment(segment_t segment , ptr_t new_point);
        void set_to_data_segment(segment_t segment);

        segment_t get_code_segment(void);
        segment_t get_data_segment(void);
    }
}

#endif