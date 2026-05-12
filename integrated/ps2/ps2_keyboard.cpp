#include <ps2.hpp>
#include <kernel/input/general_input_system.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <queue.hpp>

#define DRIVER_NAME "ps2kbd"

struct special_key_status_t {
    bool lctrl_pressed;
    bool rctrl_pressed;

    bool lshift_pressed;
    bool rshift_pressed;

    bool lalt_pressed;
    bool ralt_pressed;

    bool capslock;
    bool numlock;
    bool scrlock;

    // for special keys
    bool e0_received;
}special_key_status;

bool ps2_keyboard_driver::prepare(void) {
    char_device *device = create_empty_device<char_device>();
    // interrupt : 1 (IRQ 1)
    designate_resources_count(device , 0 , 1 , 0 , 0);
    // etc resource 1 : Scan code set
    // etc resource 2 : Shift/Alt/Ctrl, etc. status
    
    memset(&special_key_status , 0 , sizeof(special_key_status));
    dev::register_device(this , device);
    input::register_input_device(device , "keyboard");
    return true;
}

/// @brief Update the special key flags 
/// @param data data given from convert_scancode
/// @param is_pressed key_up : false, key_down : true
static void update_special_key_status(byte data , bool is_pressed) {
    switch(data) {
        case INPUT_KBD_SKEY_LALT:
            special_key_status.lalt_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_RALT:
            special_key_status.ralt_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_LCTRL:
            special_key_status.lctrl_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_RCTRL:
            special_key_status.rctrl_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_LSHIFT:
            special_key_status.lshift_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_RSHIFT:
            special_key_status.rshift_pressed = is_pressed;
            break;
        case INPUT_KBD_SKEY_CAPSLOCK:
            special_key_status.capslock = is_pressed ? 
                (special_key_status.capslock ? false : true)
                : special_key_status.capslock;
            break;
        case INPUT_KBD_SKEY_NUMLOCK:
            special_key_status.numlock = is_pressed ? 
                (special_key_status.numlock ? false : true)
                : special_key_status.numlock;
            break;
        case INPUT_KBD_SKEY_SCRLOCK:
            special_key_status.scrlock = is_pressed ? 
                (special_key_status.scrlock ? false : true)
                : special_key_status.scrlock;
            break;
    }
    return;
}

#include <scancode_translation.hpp>

Registers *ps2::ps2_interrupt_handler_irq1(Registers *regs) {
    byte scancode = io_read_byte(PS2_DATA_PORT);
    if(scancode == 0xFA
    || scancode == 0xAA
    || scancode == 0xEE) return regs;
    
    // E0 : extra
    if(scancode == 0xE0) {
        special_key_status.e0_received = true;
        return regs;
    }
    // PS/2 has only one device
    char_device *dev = (char_device *)dev::search_device(DRIVER_NAME , 0);
    if(dev == 0x00) return regs;
    
    // To-do : Scan Code set detection
    auto [data , is_special] = convert_scancode_1(scancode);
    max_t type = ((scancode & 0x80) == 0x80) ? INPUT_TYPE_KEYUP : INPUT_TYPE_KEYDOWN;;
    if(is_special) type = type == INPUT_TYPE_KEYDOWN ? INPUT_TYPE_KEYDOWN_SPECIAL : INPUT_TYPE_KEYUP_SPECIAL;

    update_special_key_status(data , !((scancode & 0x80) == 0x80));

    input_event event(type , data);
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