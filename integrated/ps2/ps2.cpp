#include <ps2.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <x86_64/pic.hpp>
#include <arch_inline_asm.hpp>

#include <registers.hpp>
#include <kernel/mem/kmem_manager.hpp>

static bool is_ps2_initialized = false;

bool ps2::output_buffer_status(void) { return ((io_read_byte(PS2_STATUS_PORT) & 0b01) == 0b01); }
bool ps2::input_buffer_status(void) { return ((io_read_byte(PS2_STATUS_PORT) & 0b10) == 0b10); }

void ps2::flush_output_buffer(void) {
    while(output_buffer_status() == PS2_FULL) { io_read_byte(PS2_DATA_PORT); }
}

bool ps2::wait_for_output_buffer_status(int status) {
    for(int i = 0; i < 0xffffffff; i++) { if(output_buffer_status() == status) return true; }
    return false;
}

bool ps2::wait_for_input_buffer_status(int status) {
    for(int i = 0; i < 0xffffffff; i++) { if(input_buffer_status() == status) return true; }
    return false;
}

byte ps2::read_controller_configuration_byte(void) {
    io_write_byte(PS2_COMMAND_PORT , 0x20); // read the controller configuration byte
    wait_for_output_buffer_status(PS2_FULL);
    return io_read_byte(PS2_DATA_PORT);
}

void ps2::write_controller_configuration_byte(byte configuration_byte) {
    io_write_byte(PS2_COMMAND_PORT , 0x60); // write the controller configuration byte
    wait_for_input_buffer_status(PS2_EMPTY);
    io_write_byte(PS2_DATA_PORT , configuration_byte);
}

void enable_mouse(void) {
    byte ACK , compaq_status;
    io_write_byte(PS2_COMMAND_PORT , 0xA8); // enable auxiliary mouse device
    ps2::flush_output_buffer();

    io_write_byte(PS2_COMMAND_PORT , 0x20);
    ps2::wait_for_output_buffer_status(PS2_FULL);

    compaq_status = io_read_byte(PS2_DATA_PORT); // read the compaq status
    debug::out::printf("compaq_status = 0x%X\n" , compaq_status);
    compaq_status |= (1 << 1); // enale IRQ12 interrupt
    
    ps2::wait_for_input_buffer_status(PS2_EMPTY); // write the compaq status
    io_write_byte(PS2_COMMAND_PORT , 0x60);
    ps2::wait_for_input_buffer_status(PS2_EMPTY);
    io_write_byte(PS2_DATA_PORT , compaq_status);

    ps2::wait_for_input_buffer_status(PS2_EMPTY); // set the settings to default
    io_write_byte(PS2_COMMAND_PORT , 0xD4); 
    ps2::wait_for_input_buffer_status(PS2_EMPTY);
    io_write_byte(PS2_DATA_PORT , 0xF6);
    ps2::wait_for_output_buffer_status(PS2_FULL);
    io_read_byte(PS2_DATA_PORT);

    ps2::wait_for_input_buffer_status(PS2_EMPTY); // enable data packet streaming
    io_write_byte(PS2_COMMAND_PORT , 0xD4); 
    ps2::wait_for_input_buffer_status(PS2_EMPTY);
    io_write_byte(PS2_DATA_PORT , 0xF4);
    debug::out::printf("Enabling data packet streaming\n");

    // read ACK
    ps2::wait_for_output_buffer_status(PS2_FULL);
    if((ACK = io_read_byte(PS2_DATA_PORT)) != 0xFA) {
        debug::out::printf("Error!\n");
        return;
    }
}

bool ps2::initialize(void) {
    if(is_ps2_initialized == true) return true;
    interrupt::hardware::disable();

    byte self_test_result , configuration_byte;
    flush_output_buffer();

    // 1. Disable Devices
    io_write_byte(PS2_COMMAND_PORT , 0xAD); // disable first PS/2 port
    io_write_byte(PS2_COMMAND_PORT , 0xA7); // disable second PS/2 port

    // 2. Flush the output buffer
    flush_output_buffer();

    // 3. Set the controller's configuration byte
    configuration_byte = read_controller_configuration_byte();
    debug::out::printf("configuration_byte = 0x%X\n" , configuration_byte);
    configuration_byte &= 0b0000000;
    write_controller_configuration_byte(configuration_byte); // write the controller configuration byte

    // 4. Perform controller self-test
    io_write_byte(PS2_COMMAND_PORT , 0xAA); // controller self-test
    wait_for_output_buffer_status(PS2_FULL);
    if((self_test_result = io_read_byte(PS2_DATA_PORT)) != 0x55) {
        debug::out::printf("self-test failed! result = 0x%X\n" , self_test_result);
        interrupt::hardware::enable();
        return false;
    }
    debug::out::printf("self-test pass\n");
    
    // 5. Check whether it is a dual channel
    io_write_byte(PS2_COMMAND_PORT , 0xA8);
    configuration_byte = read_controller_configuration_byte();
    if((configuration_byte & (1 << 5)) == (1 << 5)) {
        debug::out::printf("Dual channel not supported!\n");
    }

    // 6. Interface Test
    io_write_byte(PS2_COMMAND_PORT , 0xAB); // check first PS/2 port
    wait_for_output_buffer_status(PS2_FULL);
    if((self_test_result = io_read_byte(PS2_DATA_PORT)) != 0x00) {
        debug::out::printf("first PS/2 port self-test failed! result = 0x%X\n" , self_test_result);
        interrupt::hardware::enable();
        return false;
    }
    debug::out::printf("first ps/2 port interface-test passed\n");
    io_write_byte(PS2_COMMAND_PORT , 0xA9); // check second PS/2 port
    wait_for_output_buffer_status(PS2_FULL);
    if((self_test_result = io_read_byte(PS2_DATA_PORT)) != 0x00) {
        debug::out::printf("second PS/2 port self-test failed! result = 0x%X\n" , self_test_result);
        interrupt::hardware::enable();
        return false;
    }
    debug::out::printf("second ps/2 port interface-test passed\n");

    // 7. Enable devices
    io_write_byte(PS2_COMMAND_PORT , 0xAE); // enable port 1
    io_write_byte(PS2_COMMAND_PORT , 0xA8); // enable port 2
    
    configuration_byte = read_controller_configuration_byte();
    configuration_byte |= ((1 << 0)|(1 << 1)|(1 << 6)); // enable interrupt for both port 1 and port 2
    write_controller_configuration_byte(configuration_byte);

    // 8. Reset devices
    io_write_byte(PS2_DATA_PORT , 0xFF);
    wait_for_output_buffer_status(PS2_FULL);
    byte ACK = io_read_byte(PS2_DATA_PORT);
    if(ACK != 0xFA && ACK != 0xAA) {
        debug::out::printf("reset first port failed! result = 0x%X\n" , ACK);
        interrupt::hardware::enable();
        return false;
    }
    
    io_write_byte(PS2_COMMAND_PORT , 0xD4);
    wait_for_input_buffer_status(PS2_EMPTY);
    io_write_byte(PS2_DATA_PORT , 0xFF);
    wait_for_output_buffer_status(PS2_FULL);
    ACK = io_read_byte(PS2_DATA_PORT);
    if(ACK != 0xFA && ACK != 0xAA) {
        debug::out::printf("reset second port failed! result = 0x%X\n" , ACK);
        interrupt::hardware::enable();
        return false;
    }

    is_ps2_initialized = true;

    enable_mouse();
    
    initialize_interrupt();
    return true;
}

void ps2::initialize_interrupt(void) {
    interrupt::general::register_interrupt(0x20+1  , ps2_interrupt_handler_irq1 , INTERRUPT_HANDLER_HARDWARE|INTERRUPT_HANDLER_LEVEL_KERNEL);
    
    // somehow you have to unmask the interrupt number 2 to make mouse work... how weird! 
    interrupt::general::set_interrupt_mask(0x20+2 , false);
    interrupt::general::register_interrupt(0x20+12 , ps2_interrupt_handler_irq12 , INTERRUPT_HANDLER_HARDWARE|INTERRUPT_HANDLER_LEVEL_KERNEL);
}