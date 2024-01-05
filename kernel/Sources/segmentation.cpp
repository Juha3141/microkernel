#include <segmentation.hpp>
#include <segmentation_hardware.hpp>
#include <kmem_manager.hpp>
#include <kernel_info.hpp>

#include <string.hpp>
#include <debug.hpp>

void segmentation::SegmentsManager::init(int entry_count) {
    segments_data = (s_data_t *)memory::kstruct_alloc(entry_count*sizeof(s_data_t));
    for(int i = 0; i < entry_count; i++) {
        segments_data[i].occupied = false;
    }
    segments_count = 0;
    segments_maxcount = entry_count;
}

void segmentation::SegmentsManager::register_segment(const char *segment_name , segment_t segment_value) {
    debug::push_function("SegMgr::reg_seg");
    int index = 0;
    for(; index < segments_maxcount; index++) {
        if(segments_data[index].occupied == false) break;
    }
    if(index >= segments_maxcount) {
        // warning
        debug::pop_function();
        return;
    }
    debug::out::printf("registering segment \"%s\" to index %d -- " , segment_name , index);
    segments_data[index].occupied = true;
    strcpy(segments_data[index].name , segment_name);
    debug::out::raw_printf("%s\n" , segments_data[index].name);
    segments_data[index].value = segment_value;
    segments_count++;
    
    debug::pop_function();
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
    struct kernel_segments_info kseginfo;
    struct kernel_segments_value ksegvalue;
    SegmentsManager *segment_mgr = SegmentsManager::get_self();
    segment_mgr->init(SEGMENT_MAXCOUNT);
    
    segmentation::hardware::set_essential_kernel_segment(kseginfo);

    // Initialize both hardware/software
    segmentation::hardware::init(kseginfo , ksegvalue);
    segment_mgr->register_segment(SEGMENT_NAME_CODE , ksegvalue.kernel_code);
    segment_mgr->register_segment(SEGMENT_NAME_DATA , ksegvalue.kernel_data);
    debug::out::printf("segment_mgr : 0x%lX\n" , (max_t)segment_mgr);

    debug::out::printf(DEBUG_SPECIAL , "current segment count : %d\n" , segment_mgr->segments_count);
    for(int i = 0; i < segment_mgr->segments_maxcount; i++) {
        if(segment_mgr->segments_data[i].occupied == false) continue;
        debug::out::printf(DEBUG_SPECIAL , "%d. \"%s\", value:0x%02x\n" , i , segment_mgr->segments_data[i].name , segment_mgr->segments_data[i].value);
    }

    set_to_code_segment(SEGMENT_NAME_CODE);
    set_to_data_segment(SEGMENT_NAME_DATA);
    debug::pop_function();
}
    
segment_t segmentation::get_segment_value(const char *segment_name) {
    return SegmentsManager::get_self()->search_segment(segment_name);
}

void segmentation::discard_segment(const char *segment_name) {
    segment_t segment = SegmentsManager::get_self()->discard_segment(segment_name);
    segmentation::hardware::discard_segment(segment);
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