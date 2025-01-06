#ifndef _PS2_HPP_
#define _PS2_HPP_

#include <kernel/driver/char_device_driver.hpp>

#define PS2_COMMAND_PORT 0x64
#define PS2_STATUS_PORT  0x64
#define PS2_DATA_PORT    0x60

struct ps2_keyboard_driver : chardev::char_device_driver {
    bool prepare(void) override;
    bool open(chardev::char_device *device) override;
    bool close(chardev::char_device *device) override;
    max_t read(chardev::char_device *device , void *buffer , max_t size) override;
    max_t write(chardev::char_device *device , void *buffer , max_t size) override;
    bool io_read(chardev::char_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(chardev::char_device *device , max_t command , max_t argument) override;
};

struct ps2_mouse_driver : chardev::char_device_driver {
    bool prepare(void) override;
    bool open(chardev::char_device *device) override;
    bool close(chardev::char_device *device) override;
    max_t read(chardev::char_device *device , void *buffer , max_t size) override;
    max_t write(chardev::char_device *device , void *buffer , max_t size) override;
    bool io_read(chardev::char_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(chardev::char_device *device , max_t command , max_t argument) override;
};

#define PS2_EMPTY 0
#define PS2_FULL  1

namespace ps2 {
    bool initialize(void);
    void initialize_interrupt(void);

    bool output_buffer_status(void);
    bool input_buffer_status(void);

    void flush_output_buffer(void);
    bool wait_for_output_buffer_status(int status);
    bool wait_for_input_buffer_status(int status);

    byte read_controller_configuration_byte(void);
    void write_controller_configuration_byte(byte configuration_byte);

    
    void ps2_interrupt_handler_irq1(struct Registers *regs);
    void ps2_interrupt_handler_irq12(struct Registers *regs);
}

#endif