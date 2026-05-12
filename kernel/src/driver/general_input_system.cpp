#include <kernel/input/general_input_system.hpp>
#include <kernel/driver/device_driver.hpp>
#include <kernel/mem/kmem_manager.hpp>

#define INPUT_DEVICE_QUEUE_SIZE 512

FixedArray<input_device_collection*> *inputdev_collection_mgr;
FixedArray<input_device*> *unified_input_device_container;

void input::init() {
    inputdev_collection_mgr        = memory::new_global_object<FixedArray<input_device_collection*>>();
    inputdev_collection_mgr->init(512);
    unified_input_device_container = memory::new_global_object<FixedArray<input_device*>>();
    unified_input_device_container->init(512);

    register_input_type("keyboard");
    register_input_type("mouse");
}

/// @brief Registers a new type of input device. 
///        Specifically, create a new input_device_collection struct and registers it to kernel manager
/// @param input_type Name of the input type
/// @return ID of the input_device_collection structure
max_t input::register_input_type(const char *input_type) {
    input_device_collection *collection = (input_device_collection *)memory::pmem_alloc(sizeof(input_device_collection));
    collection->init();
    collection->id = inputdev_collection_mgr->add(collection);
    collection->registered_input_readers.init();
    strncpy(collection->name , input_type , 24);
    return collection->id;
}

input_device *input::register_input_device(max_t driver_id , max_t device_id , const char *input_type) {
    input_device *inputdev = (input_device *)memory::pmem_alloc(sizeof(input_device));

    inputdev->inputdev_id = unified_input_device_container->add(inputdev);
    inputdev->driver_id = driver_id;
    inputdev->device_id = device_id;
    inputdev->input_queue.init(INPUT_DEVICE_QUEUE_SIZE);
    inputdev->registered_input_readers.init();

    // Search the input collection from the given input_type
    max_t id = inputdev_collection_mgr->search([input_type](const input_device_collection *col) {
        return (strcmp(col->name , input_type) == 0);
    });
    inputdev->collection_ptr = inputdev_collection_mgr->get(id); 
    inputdev->collection_ptr->add_rear(inputdev);
    return inputdev;
}

input_device *input::register_input_device(general_device *device , const char *input_type) {
    max_t device_id = device->id;
    max_t driver_id = device->driver->driver_id;
    return register_input_device(driver_id , device_id , input_type);
}

/// @brief Report input to all the InputReader registered for collection named "input_type"
void input::report_input(const char *input_type , const input_event &event) {
    max_t collection_id = inputdev_collection_mgr->search([input_type](const input_device_collection *col) {
        return (strcmp(col->name , input_type) == 0);
    });
    if(collection_id == INVALID) return;

    auto collection = inputdev_collection_mgr->get(collection_id);
    report_input(collection , event);
}

void input::report_input(input_device_collection *collection , const input_event &event) {
    auto inputreader_ll = collection->registered_input_readers;
    auto ptr = inputreader_ll.get_start_node();

    // fill out the information of where the input came from
    input_event event_copy = event;
    event_copy.input_type_id = collection->id;

    while(ptr != nullptr) {
        ptr->object->event_queue.enqueue(event_copy);

        ptr = ptr->next;
    }
}

/// @brief Reports the input event, also reports input to the collection queue
/// @param device 
/// @param event 
void input::report_input(general_device *device , const input_event &event) {
    max_t id = unified_input_device_container->search([device](const input_device *dev) {
        return (dev->driver_id == device->driver->driver_id && dev->device_id == device->id);
    });
    if(id == INVALID) return;
    report_input(id , event);
}

void input::report_input(max_t inputdev_id , const input_event &event) {
    auto *input_dev = unified_input_device_container->get(inputdev_id);
    auto inputreader_ll = input_dev->registered_input_readers;

    input_event event_copy = event;
    event_copy.input_type_id = input_dev->collection_ptr->id;
    event_copy.inputdev_id   = input_dev->inputdev_id;

    auto ptr = inputreader_ll.get_start_node();
    while(ptr != nullptr) {
        ptr->object->event_queue.enqueue(event_copy);

        ptr = ptr->next;
    }
    report_input(input_dev->collection_ptr , event_copy);
}

input_device *input::search_input_device(max_t driver_id , max_t device_id) {
    max_t id = unified_input_device_container->search([driver_id,device_id](const input_device *idev) {
        return idev->driver_id == driver_id && idev->device_id == device_id;
    });
    if(id == INVALID) return nullptr;
    return unified_input_device_container->get(id);
}

input_device *input::search_input_device(max_t idev_id) {
    if(unified_input_device_container->get_max_size() <= idev_id) return nullptr;

    return unified_input_device_container->get(idev_id);
}


bool InputReader::open(max_t driver_id , max_t device_id) {
    auto *idev = input::search_input_device(driver_id , device_id);
    if(idev == nullptr) return false;
    this->inputdev = idev;

    this->node_ptr = idev->registered_input_readers.add_rear(this);
    event_queue.init(512);
    return true;
}

bool InputReader::open(max_t inputdev_id) {
    auto *idev = input::search_input_device(inputdev_id);
    if(idev == nullptr) return false;
    this->inputdev = idev;

    this->node_ptr = idev->registered_input_readers.add_rear(this);
    event_queue.init(512);
    return true;
}

bool InputReader::open(const char *input_type) {
    max_t idev_id = inputdev_collection_mgr->search([input_type](const input_device_collection *col) {
        return (strcmp(col->name , input_type) == 0);
    });
    is_inside_collection = true;
    if(idev_id == INVALID) return false;

    this->inputdev_collection = inputdev_collection_mgr->get(idev_id);
    this->node_ptr = this->inputdev_collection->registered_input_readers.add_rear(this);
    event_queue.init(512);
    return true;
}

bool InputReader::read(input_event &event) { return event_queue.dequeue(event); }
int InputReader::available_event_count()   { return event_queue.queue_size(); }

void InputReader::close() {
    if(is_inside_collection) inputdev_collection->registered_input_readers.remove(this->node_ptr);
    else inputdev->registered_input_readers.remove(this->node_ptr);
    
    inputdev_collection = nullptr;
    inputdev = nullptr;
    event_queue.flush();
}