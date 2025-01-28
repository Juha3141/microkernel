#include <ps2.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <queue.hpp>

static max_t ps2_keyboard_driver_id;

bool ps2_keyboard_driver::prepare(void) {
    chardev::char_device *device = create_empty_device<chardev::char_device>();
    // interrupt : 1 (IRQ 1)
    // etc resource : scan code queue
    designate_resources_count(device , 0 , 1 , 0 , 1);
    Queue<byte>*scan_code_queue = (Queue<byte>*)memory::pmem_alloc(sizeof(Queue<byte>));
    
    scan_code_queue->init(1024);

    device->resources.etc_resources[0] = (max_t)scan_code_queue;
    // device->resources.etc_resources[1] = (max_t)key_pressed_status;

    chardev::register_device(ps2_keyboard_driver_id , device);
    return true;
}

void ps2::ps2_interrupt_handler_irq1(struct Registers *regs) {
    byte data = io_read_byte(PS2_DATA_PORT);
    if(data == 0xFA) return;

    chardev::char_device *dev = chardev::search_device(ps2_keyboard_driver_id , 0);
    if(dev == 0x00) return;

    Queue<byte>*scan_code_queue = (Queue<byte>*)dev->resources.etc_resources[0];
    scan_code_queue->enqueue(data);
    debug::out::printf("K : 0x%X\n" , data);
}

bool ps2_keyboard_driver::open(chardev::char_device *device) { 
    return true;
}

bool ps2_keyboard_driver::close(chardev::char_device *device) {
    return true;
}

max_t ps2_keyboard_driver::read(chardev::char_device *device , void *buffer , max_t size) { 
    
    return 0;
}

// you cannot write to keyboard
max_t ps2_keyboard_driver::write(chardev::char_device *device , void *buffer , max_t size) { 
    return 0;
}

bool ps2_keyboard_driver::io_read(chardev::char_device *device , max_t command , max_t argument , max_t &data_out) { 
    return 0;
}

bool ps2_keyboard_driver::io_write(chardev::char_device *device , max_t command , max_t argument) { 
    return 0;
}


static void init_ps2_keyboard_driver(void) {
    ps2::initialize();
    ps2_keyboard_driver_id = chardev::register_driver(new ps2_keyboard_driver , "ps2kbd");
}

INIT_DEVICE_DRIVER(init_ps2_keyboard_driver)