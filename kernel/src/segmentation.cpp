#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>
#include <arch/segmentation_hardware.hpp>

#include <string.hpp>

FixedArray<segmentation::segment_info_t> *segment_mgr;

#ifdef CONFIG_USE_SEGMENTATION

void segmentation::init(void) {
    // default kernel segments
    struct kernel_segments_info kseginfo;
    segment_mgr = memory::new_global_object<FixedArray<segmentation::segment_info_t>>();
    segment_mgr->init(CONFIG_SEGMENTATION_SEGMENT_MAXCOUNT);
    
    /* Use flat model for kernel segment */
    kseginfo.kernel_code.start_address = 0x00;
    kseginfo.kernel_code.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_code.segment_type = SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_FLAT;
    kseginfo.kernel_code.privilege = SEGMENT_PRIVILEGE_KERNEL;
    
    kseginfo.kernel_data.start_address = 0x00;
    kseginfo.kernel_data.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_data.segment_type = SEGMENT_TYPE_SYSTEM_SEGMENT|SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_FLAT;
    kseginfo.kernel_data.privilege = SEGMENT_PRIVILEGE_KERNEL;

    // Initialize both hardware/software
    auto [code_segment_value , data_segment_value] = segmentation::hardware::init(kseginfo);
    kseginfo.kernel_code.value = code_segment_value;
    kseginfo.kernel_data.value = data_segment_value;

    segment_mgr->add(kseginfo.kernel_code);
    segment_mgr->add(kseginfo.kernel_data);

    set_to_code_segment(SEGMENT_ID_KERNEL_CODE);
    set_to_data_segment(SEGMENT_ID_KERNEL_DATA);
}
    
bool segmentation::get_segment_info(max_t id , segmentation::segment_info_t &segment_info) {
    return segment_mgr->get(id , segment_info);
}
    
bool segmentation::get_segment_info_from_val(segment_t segment_value , segmentation::segment_info_t &segment_info) {
    max_t id = segment_mgr->search(
        [segment_value](segment_info_t &data){ return (bool)(data.value == segment_value); }
    );
    if(id == INVALID) return false;
    memcpy(&segment_info , &(segment_mgr[id]) , sizeof(segment_info_t));
    return true;
}

segment_t segmentation::get_segment_value(max_t id) {
    segment_info_t seginfo;
    if(!segment_mgr->get(id , seginfo)) return INVALID;

    return seginfo.value;
}

max_t segmentation::create_segment(max_t start_address , max_t length , word segment_type , word privilege) {
    segment_t segment;
    segment_info_t info = {
        .start_address = start_address , 
        .length = length , 
        .segment_type = segment_type , 
        .privilege = privilege , 
    };
    if((segment_type & SEGMENT_TYPE_SYSTEM_SEGMENT) == SEGMENT_TYPE_SYSTEM_SEGMENT) {
        segment = segmentation::hardware::register_system_segment(start_address , length , segment_type , privilege);
    }
    else if((segment_type & SEGMENT_TYPE_TASK_SEGMENT) == SEGMENT_TYPE_TASK_SEGMENT) {
        segment = segmentation::hardware::register_task_segment(start_address , length , segment_type , privilege);
    }
    if(segment == SEGMENT_VALUE_INVALID) return false;
    info.value = segment;

    return segment_mgr->add(info);
}

bool segmentation::discard_segment(max_t id) {
    segment_info_t info;
    if(!segment_mgr->get(id , info)) return false;

    segment_mgr->discard(id);
    segmentation::hardware::discard_segment(info.value);
    return true;
}

void segmentation::set_to_code_segment(max_t id , ptr_t new_point) {
    segment_t segment = get_segment_value(id);
    if(segment == INVALID) return;

    if(new_point == ARCHITECTURE_LIMIT) segmentation::hardware::set_to_code_segment(segment);
    else segmentation::hardware::set_to_code_segment(segment , new_point);
}

void segmentation::set_to_data_segment(max_t id) {
    segment_t segment = get_segment_value(id);
    if(segment == INVALID) return;

    segmentation::hardware::set_to_data_segment(segment);
}

#else

void segmentation::init(void) {}
bool segmentation::get_segment_info(const char *segment_name , segmentation::segment_info_t &segment_info) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); return false; }
bool segmentation::get_segment_info(segment_t segment_value , segmentation::segment_info_t &segment_info) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); return false; }
segment_t segmentation::get_segment_value(const char *segment_name) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); return 0x00; }
bool segmentation::register_segment(const char *segment_name , max_t start_address , max_t length , word segment_type , max_t task_id) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); return false; }
bool segmentation::discard_segment(const char *segment_name) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); return false; }
void segmentation::set_to_code_segment(const char *segment_name , ptr_t new_point) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); }
void segmentation::set_to_data_segment(const char *segment_name) { debug::out::printf("Unimplemented function called : Segmentation feature is disabled\n"); }

#endif