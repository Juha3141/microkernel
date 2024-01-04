#ifndef _SEGMENTATION_HARDWARE_HPP_
#define _SEGMENTATION_HARDWARE_HPP_

#include <segmentation.hpp>

namespace segmentation {
    namespace hardware {
        void customize_segmentation_model(kernel_segments_info &kseginfo);
        
        void init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues);
        void init_ist(void); // If necessary

        segment_t register_segment(max_t start_address , max_t length , word segment_type);
        void discard_segment(segment_t segment);

        void set_to_code_segment(segment_t segment , ptr_t new_point);
        void set_to_data_segment(segment_t segment);
    }
}

#endif