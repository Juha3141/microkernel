#include <pic.hpp>
#include <io_port.hpp>

void x86_64::pic::init(void) {
    // remap PIC

    // For Master : 
    io_write_byte(PIC_MASTER_COMMAND , PIC_ICW1_INIT|PIC_ICW1_ICW4_P);
    io_write_byte(PIC_MASTER_DATA , 32); // start vector of interrupt (Master)
    io_write_byte(PIC_MASTER_DATA , 0b00000100); // IRQ2 : Where Slave is connected
    io_write_byte(PIC_MASTER_DATA , PIC_ICW4_8086);

    // For Slave : 
    io_write_byte(PIC_SLAVE_COMMAND , PIC_ICW1_INIT|PIC_ICW1_ICW4_P);
    io_write_byte(PIC_SLAVE_DATA , 32+8); // start vector of interrupt (Slave)
    io_write_byte(PIC_SLAVE_DATA , 0b00000010); // ??
    io_write_byte(PIC_SLAVE_DATA , PIC_ICW4_8086);

    io_write_byte(PIC_MASTER_DATA , 0x00);
    io_write_byte(PIC_SLAVE_DATA , 0x00);
}

void x86_64::pic::disable(void) {
    io_write_byte(PIC_MASTER_DATA , 0xFF);
    io_write_byte(PIC_SLAVE_DATA , 0xFF);
}

void x86_64::pic::irq_mask(int irq) {
    word port = (irq > 8) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;
    irq = (irq > 8) ? irq-8 : irq;
    byte value = io_read_byte(port);
    
    value |= (1 << irq);

    io_write_byte(port , value);
}

void x86_64::pic::irq_unmask(int irq) {
    word port = (irq > 8) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;
    irq = (irq > 8) ? irq-8 : irq;
    byte value = io_read_byte(port);
    
    value &= ~(1 << irq);

    io_write_byte(port , value);
}

void x86_64::pic::send_eoi_master(void) {
    io_write_byte(PIC_MASTER_COMMAND , 0x20);
}

void x86_64::pic::send_eoi_slave(void) {
    io_write_byte(PIC_SLAVE_COMMAND , 0x20);
}