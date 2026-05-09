#ifndef _GENERAL_INPUT_SYSTEM_HPP_
#define _GENERAL_INPUT_SYSTEM_HPP_

#include <kernel/driver/device_driver.hpp>
#include <kernel/driver/char_device_driver.hpp>
#include <queue.hpp>

#define INPUT_TYPE_KEYUP   
#define INPUT_TYPE_KEYDOWN 

/// @brief Rudimentary input event
struct input_event {
    max_t type;
    max_t data;

    // no need to fill in
    max_t inputdev_id;
    max_t input_type_id;
};

struct input_device;
class InputReader;

/// @brief Collection of input devices with same "type"
class input_device_collection : public LinkedList<input_device*> {
public:
    // Identifier finding out the collection
    max_t id;
    char name[24];

    // Unified event queue
    Queue<input_event>        unified_input_event_queue;
    LinkedList<InputReader*>  registered_input_readers;
};

struct input_device {
    max_t inputdev_id;
    max_t driver_id , device_id;

    input_device_collection *collection_ptr;
    Queue<input_event>       input_queue;
    LinkedList<InputReader*> registered_input_readers;
};

namespace input {
    void init();
    max_t register_input_type(const char *input_type);
    /// @brief Register the input device from device pointer
    /// @param device 
    /// @return Returns the input device id 
    input_device *register_input_device(max_t driver_id , max_t device_id , const char *input_type);
    input_device *register_input_device(general_device *device , const char *input_type);

    void report_input(input_device_collection *collection , const input_event &event);
    void report_input(const char *input_type , const input_event &event);
    void report_input(general_device *device , const input_event &event);
    void report_input(max_t inputdev_id , const input_event &event);

    input_device *search_input_device(max_t driver_id , max_t device_id);
    input_device *search_input_device(max_t idev_id);
}

class InputReader {
public:
    InputReader() = default;
    bool open(max_t driver_id , max_t device_id);
    bool open(const char *input_type); // probably most useful
    bool open(max_t inputdev_id);

    bool read(input_event &event);
    int available_event_count();
    void close();
private:
    friend void input::report_input(input_device_collection *collection , const input_event &event);
    friend void input::report_input(const char *input_type , const input_event &event);
    friend void input::report_input(max_t inputdev_id , const input_event &event);
    friend void input::report_input(general_device *device , const input_event &event);
    // stuff that will actually take care of queue things
    bool is_inside_collection = false;
    input_device *inputdev = nullptr;
    input_device_collection *inputdev_collection = nullptr;
    LinkedList<InputReader*>::node_s *node_ptr;
    
    Queue<input_event>event_queue;
};

#endif