#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <arch/segmentation_hardware.hpp>

#include <string.hpp>

bool segmentation::SegmentsManager::register_segment(const char *segment_name , segment_t segment_value , word segment_type , max_t task_id) {
    max_t index = register_space();
    if(index == INVALID) {
        return false;
    }
    strcpy(data_container[index].data.name , segment_name);
    data_container[index].data.value = segment_value;

    if(((segment_type & SEGMENT_TYPE_TASK_SEGMENT) == SEGMENT_TYPE_TASK_SEGMENT) && (task_id == INVALID)) {
        debug::out::printf(DEBUG_WARNING , "Undetermined/invalid task id given to task segment(\"%s\")\n" , segment_name);
        return false;
    }
    return true;
}

static bool checkfunc(segmentation::segment_info_t &data , const char *name) {
    return (strcmp(data.name , name) == 0);
}

segment_t segmentation::SegmentsManager::discard_segment(const char *segment_name) {
    segment_t value;
    max_t id = search<const char *>([](segment_info_t &data , const char *name){ return (bool)(strcmp(data.name , name) == 0);} , segment_name);
    value = get_data(id)->value;
    if(discard_space(id) == false) return 0x00;

    return value;
}

segment_t segmentation::SegmentsManager::search_segment(const char *segment_name) {
    max_t id = search<const char *>(checkfunc , segment_name);
    if(id == INVALID) return SEGMENT_VALUE_INVALID;
    return get_data(id)->value;
}

#ifdef USE_SEGMENTATION

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

    segmentation::register_segment(SEGMENT_NAME_1TO1_USER_CODE , 0x00 , ARCHITECTURE_LIMIT , SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE);
    segmentation::register_segment(SEGMENT_NAME_1TO1_USER_DATA , 0x00 , ARCHITECTURE_LIMIT , SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE);

    set_to_code_segment(SEGMENT_NAME_CODE);
    set_to_data_segment(SEGMENT_NAME_DATA);
    debug::pop_function();
}
    
bool segmentation::get_segment_info(const char *segment_name , segmentation::segment_info_t &segment_info) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    max_t id = segment_mgr->search<const char *>([](segment_info_t &data , const char *name){ return (bool)(strcmp(data.name , name) == 0);} , segment_name);
    if(id == INVALID) return false;
    memcpy(&segment_info , segment_mgr->get_data(id) , sizeof(segment_info_t));
    return true;
}
    
bool segmentation::get_segment_info(segment_t segment_value , segmentation::segment_info_t &segment_info) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    max_t id = segment_mgr->search<segment_t>([](segment_info_t &data , segment_t value){ return (bool)(data.value == value);} , segment_value);
    if(id == INVALID) return false;
    memcpy(&segment_info , segment_mgr->get_data(id) , sizeof(segment_info_t));
    return true;
}

segment_t segmentation::get_segment_value(const char *segment_name) {
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    max_t id = segment_mgr->search<const char *>([](segment_info_t &data , const char *name){ return (bool)(strcmp(data.name , name) == 0); } , segment_name);
    if(id == INVALID) return SEGMENT_VALUE_INVALID;
    return segment_mgr->get_data(id)->value;
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
    return GLOBAL_OBJECT(SegmentsManager)->register_segment(segment_name , segment , segment_type , task_id);
}

bool segmentation::discard_segment(const char *segment_name) {
    segment_t segment = GLOBAL_OBJECT(SegmentsManager)->discard_segment(segment_name);
    if(segment == SEGMENT_VALUE_INVALID) {
        return false;
    }
    segmentation::hardware::discard_segment(segment);
    return true;
}

void segmentation::set_to_code_segment(const char *segment_name , ptr_t new_point) {
    segment_t segment = GLOBAL_OBJECT(SegmentsManager)->search_segment(segment_name);
    if(new_point == ARCHITECTURE_LIMIT) segmentation::hardware::set_to_code_segment(segment);
    else segmentation::hardware::set_to_code_segment(segment , new_point);
}

void segmentation::set_to_data_segment(const char *segment_name) {
    segment_t segment = GLOBAL_OBJECT(SegmentsManager)->search_segment(segment_name);
    segmentation::hardware::set_to_data_segment(segment);
}

#else

void segmentation::init(void) {}
bool segmentation::get_segment_info(const char *segment_name , segmentation::segment_info_t &segment_info) { WARNING_NOT_USING_SEGMENTATION return false; }
bool segmentation::get_segment_info(segment_t segment_value , segmentation::segment_info_t &segment_info) { WARNING_NOT_USING_SEGMENTATION return false; }
segment_t segmentation::get_segment_value(const char *segment_name) { WARNING_NOT_USING_SEGMENTATION return 0x00; }
bool segmentation::register_segment(const char *segment_name , max_t start_address , max_t length , word segment_type , max_t task_id) { WARNING_NOT_USING_SEGMENTATION return false; }
bool segmentation::discard_segment(const char *segment_name) { WARNING_NOT_USING_SEGMENTATION return false; }
void segmentation::set_to_code_segment(const char *segment_name , ptr_t new_point) { WARNING_NOT_USING_SEGMENTATION }
void segmentation::set_to_data_segment(const char *segment_name) { WARNING_NOT_USING_SEGMENTATION }

#endif