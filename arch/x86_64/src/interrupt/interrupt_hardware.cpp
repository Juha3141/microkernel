/**
 * @file interrupt_hardware.cpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The hardware part of interrupt manager. Contains the Intel Interrupt Descriptor Table manager
 * @date 2023-12-31
 * 
 * @copyright Copyright (c) 2023 Ian Juha Ch
 * 
 */

#include <kernel/interrupt/interrupt.hpp>
#include <kernel/debug.hpp>
#include <arch/interrupt_hardware.hpp>

#include <x86_64/idt.hpp>
#include <x86_64/gdt.hpp>

#include <string.hpp>

static qword ist_address[7];
x86_64::IDTContainer *idt_container;

void interrupt::hardware::enable(void) {
    __asm__ ("sti");
}

void interrupt::hardware::disable(void) {
    __asm__ ("cli");
}

void interrupt::hardware::init(void) {
    interrupt::hardware::disable();
    
    idt_container = memory::new_global_object<x86_64::IDTContainer>();
    idt_container->init(IDT_ENTRYCOUNT);
    // Register IDTR
    max_t idtr_ptr = (max_t)&idt_container->reg;
    __asm__ ("lidt [%0]"::"r"(idtr_ptr));
    
    debug::out::printf("idtr_ptr      : 0x%X\n" , idtr_ptr);
    debug::out::printf("idt base_addr : 0x%X\n" , idt_container->entries);
}

qword interrupt::hardware::get_ist_address(void) { return ist_address[0]; }

bool interrupt::hardware::register_interrupt(int number , ptr_t handler_ptr , word interrupt_option) {
    word flags = IDT_FLAGS_P;
    byte type = 0;
    word privilege = 0;
    if(number >= CONFIG_INTERRUPT_GENERAL_MAXCOUNT) return false;
    idt_container->entries[number].base_low = handler_ptr & 0xFFFF;
    idt_container->entries[number].base_high = handler_ptr >> 16;
    if((interrupt_option & INTERRUPT_HANDLER_EXCEPTION) == INTERRUPT_HANDLER_EXCEPTION) type = IDT_TYPE_32BIT_TRAP_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_HARDWARE) == INTERRUPT_HANDLER_HARDWARE)   type = IDT_TYPE_32BIT_INTERRUPT_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_SOFTWARE) == INTERRUPT_HANDLER_SOFTWARE)   type = IDT_TYPE_32BIT_INTERRUPT_GATE;
    if((interrupt_option & INTERRUPT_HANDLER_LEVEL_KERNEL) == INTERRUPT_HANDLER_LEVEL_KERNEL) flags |= IDT_FLAGS_DPL0;
    if((interrupt_option & INTERRUPT_HANDLER_LEVEL_USER) == INTERRUPT_HANDLER_LEVEL_USER)     flags |= IDT_FLAGS_DPL3;
    idt_container->entries[number].flags = flags;
    idt_container->entries[number].type = type & 0x0F;
    idt_container->entries[number].selector = segmentation::get_segment_value(SEGMENT_ID_KERNEL_CODE); 
    idt_container->entries[number].reserved = 0x00;
    idt_container->entries[number].IST 
#ifdef CONFIG_USE_IST
     = 1;
#else
     = 0;
#endif
    return true;
}

extern x86_64::GDTContainer *gdt_container;

void interrupt::hardware::init_ist(void) {
    struct x86_64::TSS *tss = (struct x86_64::TSS *)memory::kstruct_alloc(sizeof(struct x86_64::TSS));
    int index = x86_64::gdt::register_ldt((qword)tss , sizeof(struct x86_64::TSS)-1 , GDT_TYPE_32BIT_TSS_AVAILABLE , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_G);
    gdt_container->tss_segment = (index << 3)|0; // RPL : 0
    // initialize TSS
    memset(tss , 0 , sizeof(struct x86_64::TSS));

    // Temporary Interrupt Stack Table
    tss->ist[0] = (qword)memory::pmem_alloc(2*1024*1024 , 4096)+(2*1024*1024)-8;
    debug::out::printf(DEBUG_INFO , "tss->ist[0] : 0x%X\n" , tss->ist[0]);
    tss->iopb_offset = 0xFFFF;
    ist_address[0] = (qword)tss->ist[0];
    // Set Segment
    __asm__ ("ltr %0"::"r"((word)gdt_container->tss_segment));
    
    debug::out::printf(DEBUG_TEXT , "TSS segment : 0x%X\n" , gdt_container->tss_segment);
}

bool interrupt::hardware::discard_interrupt(int number) {
    if(number >= CONFIG_INTERRUPT_GENERAL_MAXCOUNT) return false;
    memset(&(idt_container->entries[number]) , 0 , sizeof(x86_64::IDTEntry));
    return true;
}