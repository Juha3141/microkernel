#include <ps2.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <queue.hpp>

static max_t ps2_mouse_driver_id;

struct mouse_data {
    byte flags;
    byte x_movement;
    byte y_movement;
};

bool ps2_mouse_driver::prepare(void) {
    chardev::char_device *device = create_empty_device<chardev::char_device>();
    // interrupt : 1 (IRQ 12)
    // etc resource : data queue, phase
    designate_resources_count(device , 0 , 1 , 0 , 2);
    Queue<byte>*mouse_raw_data_queue = (Queue<byte> *)memory::pmem_alloc(sizeof(Queue<byte>));
    StructQueue<struct mouse_data>*mouse_data_queue = (StructQueue<struct mouse_data> *)memory::pmem_alloc(sizeof(StructQueue<struct mouse_data>));
    int *phase = (int *)memory::pmem_alloc(sizeof(int));
    mouse_data_queue->init(1024);
    mouse_raw_data_queue->init(1024);
    *phase = 0;

    device->resources.etc_resources[0] = (max_t)mouse_raw_data_queue;
    device->resources.etc_resources[1] = (max_t)mouse_data_queue;
    device->resources.etc_resources[2] = (max_t)phase;
    
    chardev::register_device(this , device);
    debug::out::printf("device id = 0x%X\n" , device->id);
    return true;
}

void ps2::ps2_interrupt_handler_irq12(struct Registers *regs) {
    byte data = io_read_byte(PS2_DATA_PORT);
    if(data == 0xFA) return; // Ignore ACK

    chardev::char_device *device = chardev::search_device(ps2_mouse_driver_id , 0x00); // since the ps/2 mouse driver has only one device
    Queue<byte>*queue_1 = (Queue<byte>*)device->resources.etc_resources[0];
    StructQueue<struct mouse_data>*queue_2 = (StructQueue<struct mouse_data>*)device->resources.etc_resources[1];
    int *phase = (int *)device->resources.etc_resources[2];

    debug::out::printf("M : %X, phase = %d\n" , data , *phase);
    queue_1->enqueue(data);
    *phase += 1;
    if((*phase%3 == 0) && (*phase != 0)) {
        byte flags , x_movement , y_movement;
        flags = queue_1->dequeue();
        x_movement = queue_1->dequeue();
        y_movement = queue_1->dequeue();
        queue_2->enqueue({flags , x_movement , y_movement});

        debug::out::printf("f=%X,x=%X,y=%X\n" , flags , x_movement , y_movement);
    }
}

bool ps2_mouse_driver::open(chardev::char_device *device) {
    return 0;
}

bool ps2_mouse_driver::close(chardev::char_device *device) { 
    return 0;
}

max_t ps2_mouse_driver::read(chardev::char_device *device , void *buffer , max_t size) {
    return 0;
}

max_t ps2_mouse_driver::write(chardev::char_device *device , void *buffer , max_t size) { 
    return 0;
}

bool ps2_mouse_driver::io_read(chardev::char_device *device , max_t command , max_t argument , max_t &data_out) { 
    return 0;
}

bool ps2_mouse_driver::io_write(chardev::char_device *device , max_t command , max_t argument) { 
    return 0;
}


static void init_ps2_mouse_driver(void) {
    ps2::initialize();
    ps2_mouse_driver_id = chardev::register_driver(new ps2_mouse_driver , "ps2mouse");
}

REGISTER_DRIVER(init_ps2_mouse_driver)