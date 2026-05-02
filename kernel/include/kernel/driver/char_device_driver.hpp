#ifndef _CHAR_DEVICE_DRIVER_HPP_
#define _CHAR_DEVICE_DRIVER_HPP_

#include <kernel/driver/device_driver.hpp>

// https://www.youtube.com/watch?v=kCAv53P1otM&t=5288s

struct char_device_driver;

struct char_device : general_device {};
struct char_device_driver : device_driver {
    virtual bool open(char_device *device) = 0;
    virtual bool close(char_device *device) = 0; 
    virtual max_t read(char_device *device , void *buffer , max_t size) = 0;
    virtual max_t write(char_device *device , void *buffer , max_t size) = 0;
};

#endif