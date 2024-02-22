#ifndef _BLOCK_DEVICE_SCHEDULER_HPP_
#define _BLOCK_DEVICE_SCHEDULER_HPP_

#include <drivers/block_device_driver.hpp>

#define BLOCK_IO_REQUEST_READ  1
#define BLOCK_IO_REQUEST_WRITE 2 

namespace bdevsched {
    struct block_io_scheduler {
        
    };
    struct io_requests {
        word req_type;

        max_t block_address;
        max_t block_size;
    };
    struct block_io_queue {
        
        int count;
        int max_count;
        io_requests *req;
    };
}

#endif