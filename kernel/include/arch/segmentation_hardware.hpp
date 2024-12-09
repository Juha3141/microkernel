#ifndef _SEGMENTATION_HARDWARE_HPP_
#define _SEGMENTATION_HARDWARE_HPP_

#include <kernel/mem/segmentation.hpp>

namespace segmentation {
    namespace hardware {
        void ARCHDEP set_essential_kernel_segment(kernel_segments_info &kseginfo);
        
        void ARCHDEP init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues);

        segment_t ARCHDEP register_system_segment(max_t start_address , max_t length , word segment_type);
        segment_t ARCHDEP register_task_segment(max_t start_address , max_t length , word segment_type);
        void ARCHDEP discard_segment(segment_t segment);

        void ARCHDEP set_to_code_segment(segment_t segment);
        void ARCHDEP set_to_code_segment(segment_t segment , ptr_t new_point);
        void ARCHDEP set_to_data_segment(segment_t segment);

        segment_t ARCHDEP get_code_segment(void);
        segment_t ARCHDEP get_data_segment(void);
    }
}

#endif