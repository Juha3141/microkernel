#include <ps2.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <queue.hpp>

#define DRIVER_NAME "ps2mouse"

struct mouse_data {
    byte flags;
    byte x_movement;
    byte y_movement;
};

bool ps2_mouse_driver::prepare(void) {
    char_device *device = create_empty_device<char_device>();
    // interrupt : 1 (IRQ 12)
    // etc resource : phase
    designate_resources_count(device , 0 , 1 , 0 , 1);
    
    dev::register_device(this , device);
    debug::out::printf("device id = 0x%X\n" , device->id);
    return true;
}

Registers *ps2::ps2_interrupt_handler_irq12(Registers *regs) {
    byte data = io_read_byte(PS2_DATA_PORT);
    if(data == 0xFA) return regs; // Ignore ACK

    
    return regs;
}

bool ps2_mouse_driver::open(char_device *device) {
    return 0;
}

bool ps2_mouse_driver::close(char_device *device) { 
    return 0;
}

max_t ps2_mouse_driver::read(char_device *device , void *buffer , max_t size) {
    return 0;
}

max_t ps2_mouse_driver::write(char_device *device , void *buffer , max_t size) { 
    return 0;
}

bool ps2_mouse_driver::io_read(general_device *device , max_t command , max_t argument , max_t &data_out) { 
    return 0;
}

bool ps2_mouse_driver::io_write(general_device *device , max_t command , max_t argument) { 
    return 0;
}


static void init_ps2_mouse_driver(void) {
    ps2::initialize();
    dev::register_driver(new ps2_mouse_driver , DRIVER_NAME , character);
}

REGISTER_DRIVER(init_ps2_mouse_driver)