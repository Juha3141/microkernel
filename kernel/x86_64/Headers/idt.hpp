#ifndef _x86_64_IDT_HPP_
#define _x86_64_IDT_HPP_

#include <x86_64_descriptor_table.hpp>

#define IDT_ENTRYCOUNT 256
#define IDT_FLAGS_DPL0                0b0000
#define IDT_FLAGS_DPL1                0b0010
#define IDT_FLAGS_DPL2                0b0100
#define IDT_FLAGS_DPL3                0b0110
#define IDT_FLAGS_P                   0b1000

#define IDT_TYPE_TASK_GATE            0x05
#define IDT_TYPE_16BIT_INTERRUPT_GATE 0x06
#define IDT_TYPE_32BIT_INTERRUPT_GATE 0x0E
#define IDT_TYPE_32BIT_TRAP_GATE      0x0F

namespace x86_64 {
    struct IDTEntry {
        word base_low;
        word selector;
        byte IST;
        byte type:4;
        byte flags:4;
        qword base_high:48;
        dword reserved;
    };
    struct IDTContainer : DescriptorTableContainer<struct IDTEntry> {
        SINGLETON_PATTERN_KSTRUCT(IDTContainer);
    };
}

#endif