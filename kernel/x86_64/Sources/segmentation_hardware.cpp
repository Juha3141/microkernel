/**
 * @file segmentation_hardware.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief x86_64 intel hardware compatiable implementation of segmentation (GDT)
 * @version 0.1
 * @date 2024-01-02
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho
 * 
 */

#include <segmentation.hpp>
#include <segmentation_hardware.hpp>
#include <interrupt.hpp>
#include <gdt.hpp>
#include <arch_inline_asm.hpp>

#include <string.hpp>
#include <debug.hpp>

void x86_64::gdt_register_raw(int index , dword base_address , dword limit , byte type , byte flags) {
    GDTContainer *gdt_container = GDTContainer::get_self();
    gdt_container->entries[index].base_low = base_address & 0xFFFFFF;
    gdt_container->entries[index].base_high = base_address >> 24;
    gdt_container->entries[index].limit_low = limit & 0xFFFF;
    gdt_container->entries[index].limit_high = limit >> 16;
    gdt_container->entries[index].flags_low = flags & 0x0F;
    gdt_container->entries[index].flags_high = flags >> 4;
    gdt_container->entries[index].type = type & 0x0F;
}

/// @brief Register TSS to global gdt container
/// @param base_address base address of tss data
/// @param limit size of tss data
/// @param type type
/// @param flags flags
/// @return index of the segment
int x86_64::gdt_register_tss(qword base_address , dword limit , byte type , byte flags) {
    GDTContainer *gdt_container = GDTContainer::get_self();
    int index = gdt_container->current_index;
    TSSEntry *tss_entry = (TSSEntry *)&(gdt_container->entries[index]);
    debug::out::printf("(tss) assigned new index : %d\n" , index);
    tss_entry->base_low = base_address & 0xFFFFFFF;
    tss_entry->base_high = base_address >> 24;
    tss_entry->limit_low = limit & 0xFFFF;
    tss_entry->limit_high = limit >> 16;
    tss_entry->flags_low = flags & 0x0F;
    tss_entry->flags_high = flags >> 4;
    tss_entry->type = type & 0x0F;
    gdt_container->current_index += 2;
    return index;
}

/// @brief memory model customizer, decides what segment goes to what address or what type it should have
/// @param kseginfo Return value, should contain the overall location/length of essential kernel segments
void segmentation::hardware::customize_segmentation_model(kernel_segments_info &kseginfo) {
    /* We use 1-to-1 corresponded memory map for x86_64 intel architecture. */
    kseginfo.kernel_code.segment_type = SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE|SEGMENT_TYPE_1TO1_CORRESPONDED;
    kseginfo.kernel_data.segment_type = SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE|SEGMENT_TYPE_1TO1_CORRESPONDED;
    kseginfo.user_code.segment_type = SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE|SEGMENT_TYPE_1TO1_CORRESPONDED;
    kseginfo.user_data.segment_type = SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_USER_PRIVILEGE|SEGMENT_TYPE_1TO1_CORRESPONDED;
}

void segmentation::hardware::init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues) {
    interrupt::hardware::disable();

    debug::push_function("segm::init");
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    gdt_container->init(GDT_ENTRYCOUNT);

    // Register null descriptor
    x86_64::gdt_register_raw(0x00 , 0x00 , 0x00 , 0x00 , 0x00);
    gdt_container->current_index = 1;
    ksegvalues.kernel_code = register_segment(kseginfo.kernel_code.start_address , kseginfo.kernel_code.length , kseginfo.kernel_code.segment_type);
    ksegvalues.kernel_data = register_segment(kseginfo.kernel_data.start_address , kseginfo.kernel_data.length , kseginfo.kernel_data.segment_type);
    ksegvalues.user_code = register_segment(kseginfo.user_code.start_address , kseginfo.user_code.length , kseginfo.user_code.segment_type);
    ksegvalues.user_data = register_segment(kseginfo.user_data.start_address , kseginfo.user_data.length , kseginfo.user_data.segment_type);
    init_ist();

    gdt_container->reg.size = gdt_container->current_index*sizeof(struct x86_64::GDTEntry);
    max_t gdtr_ptr = (max_t)&gdt_container->reg;
    debug::out::printf("gdtr_ptr      : 0x%X\n" , gdtr_ptr);
    debug::out::printf("gdt base_addr : 0x%X\n" , gdt_container->entries);

    IA ("lgdt [%0]"::"r"(gdtr_ptr));
    debug::out::printf("gdt_container->tss_segment : 0x%X\n" , gdt_container->tss_segment);
    IA ("ltr %0"::"r"((word)gdt_container->tss_segment));
    debug::pop_function();
}

void segmentation::hardware::init_ist(void) {
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    struct x86_64::TSS *tss = (struct x86_64::TSS *)memory::kstruct_alloc(sizeof(struct x86_64::TSS));
    int index = x86_64::gdt_register_tss((qword)tss , sizeof(struct x86_64::TSS)-1 , GDT_TYPE_32BIT_TSS_AVAILABLE , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_G);
    gdt_container->tss_segment = index << 3;
    // initialize TSS
    memset(tss , 0 , sizeof(struct x86_64::TSS));
    tss->ist[0] = (qword)memory::pmem_alloc(512*1024 , 4096);
    tss->iopb_offset = 0xFFFF;

    debug::out::printf("TSS segment : 0x%X\n" , gdt_container->tss_segment);
}

segment_t segmentation::hardware::register_segment(max_t start_address , max_t length , word segment_type) {
    segment_t segment_value = 0x00;
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    int index = gdt_container->current_index;
    debug::out::printf("(gdt) assigned new index : %d\n" , index);
    
    // determine real "gdt" type
    byte rpl = 0;
    byte type = GDT_TYPE_RW; // read/writeable
    byte flag = GDT_FLAGS_P|GDT_FLAGS_S|GDT_FLAGS_L; // + DPL0
    if((segment_type & SEGMENT_TYPE_CODE_SEGMENT) == SEGMENT_TYPE_CODE_SEGMENT) {
        type |= GDT_TYPE_E; // executable
    }
    if((segment_type & SEGMENT_TYPE_USER_PRIVILEGE) == SEGMENT_TYPE_USER_PRIVILEGE) {
        flag |= GDT_FLAGS_DPL3;
        rpl = 3;
    }
    if((segment_type & SEGMENT_TYPE_1TO1_CORRESPONDED) == SEGMENT_TYPE_1TO1_CORRESPONDED) {
        flag |= GDT_FLAGS_G; // granularity for limit (to cover entire 4GB area)

        // Modify the data
        start_address = 0x00;
        length = 0xFFFFF;
    }
    x86_64::gdt_register_raw(index , start_address , length , type , flag);

    segment_value = (index << 3)|rpl;
    gdt_container->current_index++;
    return segment_value;
}

void segmentation::hardware::discard_segment(segment_t segment) {
    // not supported
}

void segmentation::hardware::set_to_code_segment(segment_t segment , ptr_t new_point) {

}

void segmentation::hardware::set_to_data_segment(segment_t segment) {

}