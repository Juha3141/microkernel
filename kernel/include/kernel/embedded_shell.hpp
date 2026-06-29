#ifndef _EMBEDDED_SHELL_HPP_
#define _EMBEDDED_SHELL_HPP_

#include <kernel/mem/kmem_manager.hpp>
#include <kernel/mem/segmentation.hpp>
#include <kernel/mem/pages_manager.hpp>
#include <kernel/mem/kasan.hpp>

#include <kernel/interrupt/interrupt.hpp>
#include <kernel/interrupt/exception.hpp>
#include <kernel/io_port.hpp>
#include <kernel/driver/device_driver.hpp>
#include <kernel/driver/pci.hpp>

#include <kernel/input/general_input_system.hpp>

#include <ramdisk/ramdisk.hpp>
#include <kernel/vfs/storage_system.hpp>
#include <kernel/vfs/file_system_driver.hpp>
#include <kernel/vfs/virtual_file_system.hpp>

#include <kernel/debug.hpp>

#include <kernel/sections.hpp>
#include <loader/loader_argument.hpp>

#include <kernel/init.hpp>

namespace eshell {
    void start();

    namespace cmd {
        int help(file_t* &current_dir , int argc , char **argv);
        int cd(file_t* &current_dir , int argc , char **argv);
        int ls(file_t* &current_dir , int argc , char **argv);
        int read(file_t* &current_dir , int argc , char **argv);
        int clear(file_t* &current_dir , int argc , char **argv);
        int echo(file_t* &current_dir , int argc , char **argv);
        int mem(file_t* &current_dir , int argc , char **argv);
    }
}

#endif