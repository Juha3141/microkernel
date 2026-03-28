#ifndef _KERNEL_SETUP_ARGUMENT_HPP_
#define _KERNEL_SETUP_ARGUMENT_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/pages_manager.hpp>

struct KernelSetupArgument {
    PageTableData page_table_data;
    max_t pt_space_start;
    max_t pt_space_end;
};

#endif