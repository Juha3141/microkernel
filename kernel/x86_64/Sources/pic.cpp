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
}

void x86_64::pic::disable(void) {
    io_write_byte(PIC_MASTER_DATA , 0xFF);
    io_write_byte(PIC_SLAVE_DATA , 0xFF);
}