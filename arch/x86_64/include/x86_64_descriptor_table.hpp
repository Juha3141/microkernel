#ifndef _x86_64_DESCRIPTOR_TABLE_HPP_
#define _x86_64_DESCRIPTOR_TABLE_HPP_

#include <interface_type.hpp>
#include <segmentation.hpp>
#include <kmem_manager.hpp>
#include <string.hpp>

namespace x86_64 {
    struct DescriptorTableRegister {
        word size;
        qword base_address;
    };
    template <typename T> struct DescriptorTableContainer {
        T *entries;
        struct DescriptorTableRegister reg;
        int max_entries_count;
        segment_t tss_segment;

        void init(int entries_count) {
            word allocated_size = entries_count*sizeof(T);
            this->entries = (T *)memory::kstruct_alloc(allocated_size);
            this->reg.size = allocated_size; 
            this->reg.base_address = (qword)entries;
            memset(this->entries , 0 , allocated_size);
            
            this->max_entries_count = entries_count;
        }
    };
};

#endif