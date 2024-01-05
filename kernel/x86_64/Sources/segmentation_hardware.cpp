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

/// @brief memory model customizer, decides what segment goes to what address or what type it should have
/// @param kseginfo Return value, should contain the overall location/length of essential kernel segments
void segmentation::hardware::set_essential_kernel_segment(kernel_segments_info &kseginfo) {
    /* We use 1-to-1 corresponded memory map for x86_64 intel architecture. */
    kseginfo.kernel_code.start_address = 0x00;
    kseginfo.kernel_code.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_code.segment_type = SEGMENT_TYPE_CODE_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE;

    kseginfo.kernel_data.start_address = 0x00;
    kseginfo.kernel_data.length = ARCHITECTURE_LIMIT;
    kseginfo.kernel_data.segment_type = SEGMENT_TYPE_DATA_SEGMENT|SEGMENT_TYPE_KERNEL_PRIVILEGE;
}

void segmentation::hardware::init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues) {
    interrupt::hardware::disable();

    debug::push_function("segm::init");
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    gdt_container->init(GDT_ENTRYCOUNT);

    ksegvalues.kernel_code = register_system_segment(kseginfo.kernel_code.start_address , kseginfo.kernel_code.length , kseginfo.kernel_code.segment_type);
    ksegvalues.kernel_data = register_system_segment(kseginfo.kernel_data.start_address , kseginfo.kernel_data.length , kseginfo.kernel_data.segment_type);
    init_ist();

    gdt_container->reg.size = GDT_ENTRYCOUNT*sizeof(struct x86_64::GDTEntry);
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
    int index = x86_64::gdt::register_tss((qword)tss , sizeof(struct x86_64::TSS)-1 , GDT_TYPE_32BIT_TSS_AVAILABLE , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_G);
    gdt_container->tss_segment = (index << 3)|0; // RPL : 0
    // initialize TSS
    memset(tss , 0 , sizeof(struct x86_64::TSS));
    tss->ist[0] = (qword)memory::pmem_alloc(512*1024 , 4096)+(512*1024);
    debug::out::printf(DEBUG_INFO , "tss->ist[0] : 0x%X\n" , tss->ist[0]);
    tss->iopb_offset = 0xFFFF;

    debug::out::printf("TSS segment : 0x%X\n" , gdt_container->tss_segment);
}

segment_t segmentation::hardware::register_system_segment(max_t start_address , max_t length , word segment_type) {
    segment_t segment_value = 0x00;
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    int index = gdt_container->current_index;

    // Determine type according to segment_type
    byte type , flags , rpl;
    x86_64::gdt::convert_type_flags(segment_type , type , flags , rpl);
    if(length > 0xFFFFF) { // Enable Granularity?
        flags |= GDT_FLAGS_G; // Granularity = 1 : Multiply 4096 to limit
        length = length >> 12;
    }

    x86_64::gdt::register_raw(index , start_address , length , type , flags);
    
    segment_value = x86_64::gdt::get_segment_value(index , rpl);
    gdt_container->current_index++;
    return segment_value;
}

void segmentation::hardware::discard_segment(segment_t segment) {
    int index = x86_64::gdt::get_segment_index(segment);
    x86_64::gdt::register_raw(index , 0 , 0 , 0 , 0);
}

__attribute__ ((naked)) void segmentation::hardware::set_to_code_segment(segment_t segment) {
    IA ("mov rbx , qword[rsp-8]"); // Return address of current function (Stored in RSP)
    IA ("push %0"::"r"(segment));  // Push new code segment
    IA ("push rbx");               // Push the return address

    IA ("retfq");                  // Set the code segment and go back to return address
}

__attribute__ ((naked)) void segmentation::hardware::set_to_code_segment(segment_t segment , ptr_t new_point) {
    IA ("push %0"::"r"(segment));         // Push new code segment
    IA ("mov rax , %0"::"r"(new_point));  // Push new entry point
    IA ("push rax");

    IA ("retfq");                  // Set the code segment and jump to the address
}

__attribute__((naked)) void segmentation::hardware::set_to_data_segment(segment_t segment) {
    IA ("mov ax , %0"::"r"(segment));
    IA ("mov ds , ax");
    IA ("mov es , ax");
    IA ("mov fs , ax");
    IA ("mov gs , ax");
    IA ("mov ss , ax");

    IA_RETURN
}