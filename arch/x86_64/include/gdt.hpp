#ifndef _x86_64_GDT_HPP_
#define _x86_64_GDT_HPP_

#include <x86_64_descriptor_table.hpp>
#include <debug.hpp>

#define GDT_ENTRYCOUNT (SEGMENT_MAXCOUNT+1)
#define GDT_TYPE_RW                  0b00000010
#define GDT_TYPE_DC                  0b00000100
#define GDT_TYPE_E                   0b00001000

#define GDT_TYPE_16BIT_TSS_AVAILABLE 0x01
#define GDT_TYPE_LDT                 0x02
#define GDT_TYPE_16BIT_TSS_BUSY      0x03
#define GDT_TYPE_32BIT_TSS_AVAILABLE 0x09
#define GDT_TYPE_32BIT_TSS_BUSY      0x0B

#define GDT_FLAGS_S    0b00000001
#define GDT_FLAGS_DPL0 0b00000000
#define GDT_FLAGS_DPL1 0b00000010
#define GDT_FLAGS_DPL2 0b00000100
#define GDT_FLAGS_DPL3 0b00000110
#define GDT_FLAGS_P    0b00001000
#define GDT_FLAGS_L    0b00100000
#define GDT_FLAGS_DB   0b01000000
#define GDT_FLAGS_G    0b10000000

namespace x86_64 {
    struct GDTEntry {
        word limit_low;
        dword base_low:24;
        byte type:4;
        byte flags_low:4;
        byte limit_high:4;
        byte flags_high:4;
        byte base_high;
    };
    struct LDTEntry {
        word limit_low;
        dword base_low:24;
        byte type:4;
        byte flags_low:4;   // P[3:3] , DPL[1:2] , S[0:0]
        byte limit_high:4;  
        byte flags_high:4;  // G[3:3] , DB[2:2] , L[1:1]
        qword base_high:40;
        dword reserved;
    };
    struct TSS {
        dword reserved1;
        qword rsp[3];      // Three of RSP
        qword reserved2;
        qword ist[7];      // Seven of IST
        qword reserved3;
        word reserved4;
        word iopb_offset; // unsigned short, which means, Maximum 65535 ports are available for controlling.
    };
    typedef struct LDTEntry TSSEntry;
    struct GDTContainer : DescriptorTableContainer<struct GDTEntry> {
        int current_index;
        SINGLETON_PATTERN_KSTRUCT(GDTContainer);

        void init(int entries_count) {
            DescriptorTableContainer<struct GDTEntry>::init(entries_count);
            current_index = 1; // we skip first entry for null descriptor
        }
    };
    namespace gdt {
        void convert_type_flags(word segment_flags , byte &type , byte &flags , byte &rpl);
        segment_t get_segment_value(int index , byte rpl , bool is_ldt);
        int get_segment_index(segment_t segment_value);
        int register_gdt(dword base_address , dword limit , byte type , byte flags);
        int register_ldt(qword base_address , dword limit , byte type , byte flags);
    }
}


#endif