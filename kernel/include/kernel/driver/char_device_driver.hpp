#ifndef _CHAR_DEVICE_DRIVER_HPP_
#define _CHAR_DEVICE_DRIVER_HPP_

#include <kernel/driver/device_driver.hpp>

// https://www.youtube.com/watch?v=kCAv53P1otM&t=5288s

namespace chardev {
    class CharDeviceContainer;
    struct char_device_driver;

    struct char_device : general_device<char_device_driver> {
        // nothing yet..?
    };
    struct char_device_driver : general_device_driver<CharDeviceContainer> {
        virtual bool open(char_device *device) = 0;
        virtual bool close(char_device *device) = 0; 
        virtual max_t read(char_device *device , void *buffer , max_t size) = 0;
        virtual max_t write(char_device *device , void *buffer , max_t size) = 0;
        virtual bool io_read(char_device *device , max_t command , max_t argument , max_t &data_out) = 0;
        virtual bool io_write(char_device *device , max_t command , max_t argument) = 0;
    };

    class CharDeviceContainer : public ObjectManager<char_device> {};
    struct CharDeviceDriverContainer : public ObjectManager<char_device_driver> {
        SINGLETON_PATTERN_PMEM(CharDeviceDriverContainer);
    };
    void init(void);

    max_t register_driver(char_device_driver *driver , const char *driver_name);
    char_device_driver *search_driver(const char *driver_name);
    char_device_driver *search_driver(max_t driver_id);

    max_t discard_driver(const char *driver_name);
    max_t discard_driver(max_t driver_id);


    max_t register_device(char_device_driver *driver , char_device *device);
    max_t register_device(const char *driver_name , char_device *device);
    max_t register_device(max_t driver_id , char_device *device);
    char_device *search_device(char_device_driver *driver , max_t device_id);
    char_device *search_device(const char *driver_name , max_t device_id);
    char_device *search_device(max_t driver_id , max_t device_id);

    bool discard_device(char_device *device);

};

#endif