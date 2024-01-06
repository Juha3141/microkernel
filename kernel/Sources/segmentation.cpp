#include <segmentation.hpp>
#include <segmentation_hardware.hpp>
#include <kmem_manager.hpp>
#include <kernel_info.hpp>

#include <string.hpp>
#include <debug.hpp>

void segmentation::SegmentsManager::init(int entry_count) {
    segments_data = (segment_info_t *)memory::kstruct_alloc(entry_count*sizeof(segment_info_t));
    for(int i = 0; i < entry_count; i++) {
        segments_data[i].occupied = false;
    }
    segments_count = 0;
    segments_maxcount = entry_count;
}

bool segmentation::SegmentsManager::register_segment(const char *segment_name , segment_t segment_value , word segment_type , max_t task_id) {
    int index = 0;
    for(; index < segments_maxcount; index++) {
        if(segments_data[index].occupied == false) break;
    }
    if(index >= segments_maxcount) {
        debug::out::printf(DEBUG_WARNING , "No available space for new segment\n");
        return false;
    }
    segments_data[index].occupied = true;
    strcpy(segments_data[index].name , segment_name);
    segments_data[index].value = segment_value;
    segments_count++;

    if(((segment_type & SEGMENT_TYPE_TASK_SEGMENT) == SEGMENT_TYPE_TASK_SEGMENT) && (task_id == INVALID)) {
        debug::out::printf(DEBUG_WARNING , "Undetermined/invalid task id given to task segment(\"%s\")\n" , segment_name);
        return false;
    }
    return true;
}

segment_t segmentation::SegmentsManager::discard_segment(const char *segment_name) {
    int index = 0;
    for(; index < segments_maxcount; index++) {
        if(strcmp(segments_data[index].name , segment_name) == 0) break;
    }
    if(index >= segments_maxcount) {
        return SEGMENT_VALUE_INVALID;
    }
    segments_data[index].occupied = false;
    memset(segments_data[index].name , 0 , sizeof(segments_data[index].name));
    segments_count--;
    return segments_data[index].value;
}

segment_t segmentation::SegmentsManager::search_segment(const char *segment_name) {
    int index = 0;
    for(; index < segments_maxcount; index++) {
        if(strcmp(segments_data[index].name , segment_name) == 0) return segments_data[index].value;
    }
    return SEGMENT_VALUE_INVALID;
}

void segmentation::init(void) {
    debug::push_function("seg::init");

    // default kernel segments
    struct kernel_segments_info kseginfo;
    struct kernel_segments_value ksegvalue;
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    segment_mgr->init(SEGMENT_MAXCOUNT);
    
    /* Use 1-to-1 correspondence for kernel segment */
    kseginfo.kernel_code.start_address = 0x00;
    kseginfo.kernel_code.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_code.segment_type = SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE;
    
    kseginfo.kernel_data.start_address = 0x00;
    kseginfo.kernel_data.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_data.segment_type = SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE;

    // Initialize both hardware/software
    segmentation::hardware::init(kseginfo , ksegvalue);
    segment_mgr->register_segment(SEGMENT_NAME_CODE , ksegvalue.kernel_code , kseginfo.kernel_code.segment_type);
    segment_mgr->register_segment(SEGMENT_NAME_DATA , ksegvalue.kernel_data , kseginfo.kernel_data.segment_type);
    if(GLOBAL_KERNELINFO->conditions.use_ist == true) {
        segmentation::hardware::init_ist(GLOBAL_KERNELINFO->ist_info);
    }

    if(GLOBAL_KERNELINFO->conditions.task_segmentation_mode == KINFO_TASKSEG_LINEAR_CORRESPONDENCE) {
        segmentation::register_segment(SEGMENT_NAME_1TO1_USER_CODE , 0x00 , ARCHITECTURE_LIMIT , SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE);
        segmentation::register_segment(SEGMENT_NAME_1TO1_USER_DATA , 0x00 , ARCHITECTURE_LIMIT , SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE);
    }
    set_to_code_segment(SEGMENT_NAME_CODE);
    set_to_data_segment(SEGMENT_NAME_DATA);
    debug::pop_function();
}
    
bool segmentation::get_segment_info(const char *segment_name , segmentation::segment_info_t &segment_info) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    int index = 0;
    for(; index < segment_mgr->segments_maxcount; index++) {
        if(strcmp(segment_mgr->segments_data[index].name , segment_name) == 0) {
            memcpy(&segment_info , &segment_mgr->segments_data[index] , sizeof(segment_info_t));
            return true;
        }
    }
    return false;
}
    
bool segmentation::get_segment_info(segment_t segment_value , segmentation::segment_info_t &segment_info) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    int index = 0;
    for(; index < segment_mgr->segments_maxcount; index++) {
        if(segment_mgr->segments_data[index].value == segment_value) {
            memcpy(&segment_info , &segment_mgr->segments_data[index] , sizeof(segment_info_t));
            return true;
        }
    }
    return false;
}

segment_t segmentation::get_segment_value(const char *segment_name) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    int index = 0;
    for(; index < segment_mgr->segments_maxcount; index++) {
        if(strcmp(segment_mgr->segments_data[index].name , segment_name) == 0) {
            return segment_mgr->segments_data[index].value;
        }
    }
    return SEGMENT_VALUE_INVALID;
}

bool segmentation::register_segment(const char *segment_name , max_t start_address , max_t length , word segment_type , max_t task_id) {
    segment_t segment;
    if((segment_type & SEGMENT_TYPE_SYSTEM_SEGMENT) == SEGMENT_TYPE_SYSTEM_SEGMENT) {
        segment = segmentation::hardware::register_system_segment(start_address , length , segment_type);
    }
    else if((segment_type & SEGMENT_TYPE_TASK_SEGMENT) == SEGMENT_TYPE_TASK_SEGMENT) {
        segment = segmentation::hardware::register_task_segment(start_address , length , segment_type);
    }
    if(segment == SEGMENT_VALUE_INVALID) {
        return false;
    }
    return SegmentsManager::get_self()->register_segment(segment_name , segment , segment_type , task_id);
}

bool segmentation::discard_segment(const char *segment_name) {
    segment_t segment = SegmentsManager::get_self()->discard_segment(segment_name);
    if(segment == SEGMENT_VALUE_INVALID) {
        return false;
    }
    segmentation::hardware::discard_segment(segment);
    return true;
}

void segmentation::set_to_code_segment(const char *segment_name , ptr_t new_point) {
    segment_t segment = SegmentsManager::get_self()->search_segment(segment_name);
    if(new_point == ARCHITECTURE_LIMIT) segmentation::hardware::set_to_code_segment(segment);
    else segmentation::hardware::set_to_code_segment(segment , new_point);
}

void segmentation::set_to_data_segment(const char *segment_name) {
    segment_t segment = SegmentsManager::get_self()->search_segment(segment_name);
    segmentation::hardware::set_to_data_segment(segment);
}