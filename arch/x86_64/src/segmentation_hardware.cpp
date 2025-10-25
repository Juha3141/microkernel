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

#include <kernel/mem/segmentation.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/debug.hpp>
#include <arch/segmentation_hardware.hpp>

#include <arch_inline_asm.hpp>

#include <x86_64/gdt.hpp>

#include <string.hpp>

/// @brief Hardware-level segmentation initialization, based on kseginfo and ksegvalues
/// @param kseginfo Information for Default kernel segment 
/// @param ksegvalues Actual segment value(or anything like that) of the default kernel segment
void segmentation::hardware::init(kernel_segments_info kseginfo , kernel_segments_value &ksegvalues) {
    interrupt::hardware::disable();

    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    gdt_container->init(GDT_ENTRYCOUNT);

    ksegvalues.kernel_code = register_system_segment(kseginfo.kernel_code.start_address , kseginfo.kernel_code.length , kseginfo.kernel_code.segment_type);
    ksegvalues.kernel_data = register_system_segment(kseginfo.kernel_data.start_address , kseginfo.kernel_data.length , kseginfo.kernel_data.segment_type);

    gdt_container->reg.size = GDT_ENTRYCOUNT*sizeof(struct x86_64::GDTEntry);
    max_t gdtr_ptr = (max_t)&gdt_container->reg;
    debug::out::printf("gdtr_ptr      : 0x%X\n" , gdtr_ptr);
    debug::out::printf("gdt base_addr : 0x%X\n" , gdt_container->entries);
    debug::out::printf(DEBUG_INFO , "sizeof(GDTEntry) : %d\n" , sizeof(x86_64::GDTEntry));
    debug::out::printf(DEBUG_INFO , "sizeof(LDTEntry) : %d\n" , sizeof(x86_64::LDTEntry));

    __asm__ ("lgdt [%0]"::"r"(gdtr_ptr));
}

segment_t segmentation::hardware::register_system_segment(max_t start_address , max_t length , word segment_type) {
    segment_t segment_value = 0x00;
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    
    // Determine type according to segment_type
    byte type , flags , rpl;
    x86_64::gdt::convert_type_flags(segment_type , type , flags , rpl);
    if(length > 0xFFFFF) { // Enable Granularity?
        flags |= GDT_FLAGS_G; // Granularity = 1 : Multiply 4096 to limit
        length = length >> 12;
    }

    int index = x86_64::gdt::register_gdt(start_address , length , type , flags);
    segment_value = x86_64::gdt::get_segment_value(index , rpl , false);
    return segment_value;
}

segment_t segmentation::hardware::register_task_segment(max_t start_address , max_t length , word segment_type) {
    segment_t segment_value = 0x00;
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    
    // Determine type according to segment_type
    byte type , flags , rpl;
    x86_64::gdt::convert_type_flags(segment_type , type , flags , rpl);
    if(length > 0xFFFFF) { // Enable Granularity?
        flags |= GDT_FLAGS_G; // Granularity = 1 : Multiply 4096 to limit
        length = length >> 12;
    }

    int index = x86_64::gdt::register_ldt(start_address , length , type , flags);
    segment_value = x86_64::gdt::get_segment_value(index , rpl , false);
    return segment_value;
}

void segmentation::hardware::discard_segment(segment_t segment) {
    size_t segment_size;
    x86_64::GDTContainer *gdt_container = x86_64::GDTContainer::get_self();
    int index = x86_64::gdt::get_segment_index(segment);
    if((gdt_container->entries[index].type & GDT_TYPE_LDT) == GDT_TYPE_LDT) {
        segment_size = sizeof(x86_64::LDTEntry);
    }
    else {
        segment_size = sizeof(x86_64::GDTEntry);
    }
    memset(&(gdt_container->entries[index]) , 0 , segment_size);
}

__attribute__ ((naked)) void segmentation::hardware::set_to_code_segment(segment_t segment) {
    __asm__ ("mov rbx , [rsp]"); // Return address of current function (Stored in RSP)
    __asm__ ("push %0"::"r"((unsigned long)segment));  // Push new code segment
    __asm__ ("push rbx");               // Push the return address

    __asm__ ("retfq");                  // Set the code segment and go back to return address
}

__attribute__ ((naked)) void segmentation::hardware::set_to_code_segment(segment_t segment , ptr_t new_point) {
    __asm__ ("push %0"::"r"(segment));         // Push new code segment
    __asm__ ("push %0"::"r"((unsigned long)new_point));  // Push new entry point

    __asm__ ("retfq");                  // Set the code segment and jump to the address
}

__attribute__((naked)) void segmentation::hardware::set_to_data_segment(segment_t segment) {
    __asm__ ("mov rax , %0"::"r"(segment));
    __asm__ ("mov ds , ax");
    __asm__ ("mov es , ax");
    __asm__ ("mov fs , ax");
    __asm__ ("mov gs , ax");
    __asm__ ("mov ss , ax");

    IA_RETURN
}