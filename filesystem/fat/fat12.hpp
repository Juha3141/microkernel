#ifndef _FAT12_HPP_
#define _FAT12_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/vfs/file_system_driver.hpp>

#include "fat.hpp"
#include "fat16.hpp"

namespace fat12 {
    struct fat12_driver : fat16::fat16_driver {
        bool check(blockdev::block_device *device);
    };
    
    void register_driver(void);
};

#endif