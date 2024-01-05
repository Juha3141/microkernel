#include <gdt.hpp>

void x86_64::gdt::convert_type_flags(word segment_type , byte &type , byte &flags , byte &rpl) {
    // determine real "gdt" type
    rpl = 0; // default : kernel
    type = GDT_TYPE_RW; // read/writeable
    flags = GDT_FLAGS_P|GDT_FLAGS_S|GDT_FLAGS_L; // + DPL0
    if((segment_type & SEGMENT_TYPE_CODE_SEGMENT) == SEGMENT_TYPE_CODE_SEGMENT) {
        type |= GDT_TYPE_E; // executable
    }
    if((segment_type & SEGMENT_TYPE_USER_PRIVILEGE) == SEGMENT_TYPE_USER_PRIVILEGE) {
        flags |= GDT_FLAGS_DPL3;
        rpl = 3;
    }
}

segment_t x86_64::gdt::get_segment_value(int index , byte rpl) {
    return (index << 3)|rpl;
}

int x86_64::gdt::get_segment_index(segment_t segment_value) {
    return segment_value >> 3;
}

void x86_64::gdt::register_raw(int index , dword base_address , dword limit , byte type , byte flags) {
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
int x86_64::gdt::register_tss(qword base_address , dword limit , byte type , byte flags) {
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

    // double the size
    gdt_container->current_index += 2;
    return index;
}