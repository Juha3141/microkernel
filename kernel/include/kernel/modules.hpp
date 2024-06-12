#ifndef _MODULES_HPP_
#define _MODULES_HPP_

#include <kernel/essentials.hpp>
#include <object_manager.hpp>

namespace modules {
    typedef void(*module_init)(void);
    typedef void(*module_discard)(void);

    struct KernelModulesManager : ObjectManager<kernel_module> {
        SINGLETON_PATTERN_PMEM(KernelModulesManager);
    };

    struct kernel_module {
        max_t id;

        module_init module_init_entry;
        module_discard module_discard_entry;
    };

    void register_module(kernel_module &module);
    void discard_module(max_t module_id);
};

#endif