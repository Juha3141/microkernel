#include <ps2.hpp>
#include <kernel/driver/general_input_system.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <queue.hpp>

#define DRIVER_NAME "ps2kbd"

const char scancode_map[] = {

};

bool ps2_keyboard_driver::prepare(void) {
    char_device *device = create_empty_device<char_device>();
    // interrupt : 1 (IRQ 1)
    designate_resources_count(device , 0 , 1 , 0 , 0);
    
    dev::register_device(this , device);
    input::register_input_device(device , "keyboard");
    return true;
}

Registers *ps2::ps2_interrupt_handler_irq1(Registers *regs) {
    byte data = io_read_byte(PS2_DATA_PORT);
    if(data == 0xFA) return regs;

    // PS/2 has only one device
    char_device *dev = (char_device *)dev::search_device(DRIVER_NAME , 0);
    if(dev == 0x00) return regs;

    input_event event = {
        .data = 0 , 
        .type = 0
    };
    input::report_input(dev , event);
    return regs;
}

bool ps2_keyboard_driver::open(char_device *device) { return true; }
bool ps2_keyboard_driver::close(char_device *device) { return true; }
max_t ps2_keyboard_driver::read(char_device *device , void *buffer , max_t size) {  return 0; }

// you cannot write to keyboard
max_t ps2_keyboard_driver::write(char_device *device , void *buffer , max_t size) { 
    return 0;
}

bool ps2_keyboard_driver::io_read(general_device *device , max_t command , max_t argument , max_t &data_out) { 
    return 0;
}

bool ps2_keyboard_driver::io_write(general_device *device , max_t command , max_t argument) { 
    return 0;
}


static void init_ps2_keyboard_driver(void) {
    ps2::initialize();
    dev::register_driver(new ps2_keyboard_driver , DRIVER_NAME , character);
}

REGISTER_DRIVER(init_ps2_keyboard_driver)