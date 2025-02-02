#include "ide.hpp"

#include <kernel/driver/block_device_driver.hpp>
#include <kernel/interrupt/interrupt.hpp>
#include <kernel/io_port.hpp>
#include <kernel/debug.hpp>
#include <arch_inline_asm.hpp>

static bool primary_interrupt_flag = false;
static bool secondary_interrupt_flag = false;

void ide::interrupt_handler_irq14(struct Registers *regs) {
    main_int_handler(true);
};

void ide::interrupt_handler_irq15(struct Registers *regs) { 
    main_int_handler(false);
};

bool ide_driver::prepare(void) {
    int i;
    bool is_master_info[] = {true , false , true , false};
    io_port io_port_info[][2] = {
        {IDE_PRIMARY_BASE , IDE_DEVICECONTROL_PRIMARY_BASE} , 
        {IDE_PRIMARY_BASE , IDE_DEVICECONTROL_PRIMARY_BASE} , 
        {IDE_SECONDARY_BASE , IDE_DEVICECONTROL_SECONDARY_BASE} , 
        {IDE_SECONDARY_BASE , IDE_DEVICECONTROL_SECONDARY_BASE} , 
    };
    blockdev::block_device *devices[4]; // Four devices, Two primary, Two secondary

    debug::out::printf_function(DEBUG_INFO , "ide_driver::prepare" , "Searching every ide drives...\n");
    for(i = 0; i < 4; i++) {
        devices[i] = create_empty_device<blockdev::block_device>();
        designate_resources_count<blockdev::block_device>(devices[i] , 2 , 0 , 1 , 0);

        devices[i]->resources.io_ports[0] = io_port_info[i][0];
        devices[i]->resources.io_ports[1] = io_port_info[i][1];
        devices[i]->resources.flags[0] = is_master_info[i];

        if(blockdev::register_device(this , devices[i]) == INVALID) {
            debug::out::printf_function(DEBUG_WARNING , "ide_driver::prepare" , "Device not found in idehd%d\n" , i);
        }
        else {
            debug::out::printf_function(DEBUG_SPECIAL , "ide_driver::prepare" , "Registered device : idehd%d\n" , i);
        }
    }
    return true;
}

bool ide_driver::wait(io_port base_port) {
    byte status;
    do {
        status = io_read_byte(base_port+IDE_PORT_COMMAND_IO);
        if(((status & IDE_STATUS_ERROR) == IDE_STATUS_ERROR)) return false;
        if(status == 0x00) return false;

        if((status & IDE_STATUS_DRQ) == IDE_STATUS_DRQ) break;
    }while((status & IDE_STATUS_BUSY) == IDE_STATUS_BUSY);
    return true;
}

bool ide_driver::open(blockdev::block_device *device) {
    return true;
}

bool ide_driver::close(blockdev::block_device *device) {
    return true;
}

bool ide_driver::get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) {
    // Seperate to one getting CDROM, one getting HDD
    word status;
    word data[256];
    io_port base_port = device->resources.io_ports[0];
    ide::geometry_t ide_geometry;
    if(device->resources.io_port_count != 2) { return false; }
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    // Ports[0] : Base port
    // Ports[1] : Device control base port
    io_write_byte(base_port+IDE_PORT_SECTOR_COUNT , 0x00);
    io_write_byte(base_port+IDE_PORT_LBALOW , 0x00);
    io_write_byte(base_port+IDE_PORT_LBAMIDDLE , 0x00);
    io_write_byte(base_port+IDE_PORT_LBAHIGH , 0x00);

    io_write_byte(base_port+IDE_PORT_COMMAND_IO , 0xEC); // 0xEC : Identify
    if(ide_driver::wait(base_port) == false) return false;
    
    for(int i = 0; i < 256; i++) {
        ((word *)&(ide_geometry))[i] = io_read_word(base_port+IDE_PORT_DATA);
    }
    /*
    for(int i = 0; i < 20; i++) {
        tmp = IDEGeometry.Model[i];
        IDEGeometry.Model[i] = (Temporary >> 8)|((Temporary & 0xFF) << 8);
    }
    memcpy(Geometry->Model , IDEGeometry.Model , 20*sizeof(unsigned short));
    
    Geometry->Model[41] = 0x00;
    */
    geometry.block_size = 512;
    geometry.lba_total_block_count = ide_geometry.total_sectors;
    return true;
}

max_t ide_driver::read(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) {
    bool use_28bit_pio = true;
    io_port base_port = device->resources.io_ports[0];
    word status;
    max_t address;
    // true : master
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    // not using 28bit PIO for large block address
    if(block_address > 0x10000000) {
        io_write_byte(base_port+IDE_PORT_SECTOR_COUNT , (count >> 8) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBALOW , (block_address >> 24) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBAMIDDLE , (block_address >> 32) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBAHIGH , (block_address >> 48) & 0xFF);
        use_28bit_pio = false;
    }
    // when accessing address is below 128GB, use 28bit PIO, because it's faster than 48bit PIO
    io_write_byte(base_port+IDE_PORT_SECTOR_COUNT , count & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBALOW , block_address & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBAMIDDLE , (block_address >> 8) & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBAHIGH , (block_address >> 16) & 0xFF);
    
    // Read Sector(0x20) for 28bit, Read Sector EXT(0x24) for 48bit
    io_write_byte(base_port+IDE_PORT_COMMAND_IO , use_28bit_pio ? 0x20 : 0x24);
    for(max_t i = 0; i < count; i++) {
        if(ide_driver::wait(base_port) == false) {
            return i*512;
        }
        for(int j = 0; j < 512; j += 2) {
            address = ((max_t)buffer);
            address += ((i*512)+j);
            *((word *)address) = io_read_word(base_port+IDE_PORT_DATA);
        }
    }
    return count*512;
}

max_t ide_driver::write(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) {
    bool use_28bit_pio = true;
    io_port base_port = device->resources.io_ports[0];
    word status;
    max_t address;
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    // not using 28bit PIO for large block address
    if(block_address > 0x10000000) {
        io_write_byte(base_port+IDE_PORT_SECTOR_COUNT , (count >> 8) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBALOW , (block_address >> 24) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBAMIDDLE , (block_address >> 32) & 0xFF);
        io_write_byte(base_port+IDE_PORT_LBAHIGH , (block_address >> 48) & 0xFF);
        use_28bit_pio = false;
    }
    // when accessing address is below 128GB, use 28bit PIO, because it's faster than 48bit PIO
    io_write_byte(base_port+IDE_PORT_SECTOR_COUNT , count & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBALOW , block_address & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBAMIDDLE , (block_address >> 8) & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBAHIGH , (block_address >> 16) & 0xFF);
    
    // Write Sector(0x20) for 38bit, Read Sector EXT(0x34) for 48bit
    io_write_byte(base_port+IDE_PORT_COMMAND_IO , (use_28bit_pio == true) ? 0x30 : 0x34);
    for(int i = 0; i < count; i++) {
        if(this->wait(base_port) == false) {
            return i*512;
        }
        for(int j = 0; j < 512; j += 2) {
            address = ((max_t)buffer)+((i*512)+j);
            io_write_word(base_port+IDE_PORT_DATA , *((unsigned short *)address));
        }
    }
    return count*512;
}

bool ide_driver::io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) {
    return false;
}

bool ide_driver::io_write(blockdev::block_device *device , max_t command , max_t argument) {
    return false;
}

void ide::main_int_handler(bool is_primary) {
    if(is_primary) {
        primary_interrupt_flag = true;
        secondary_interrupt_flag = false;
    }
    else {
        primary_interrupt_flag = false;
        secondary_interrupt_flag = true;
    }
}

bool ide_cd_driver::prepare(void) {
    bool is_master_info[] = {true , false , true , false};
    io_port io_port_info[][2] = {
        {IDE_PRIMARY_BASE , IDE_DEVICECONTROL_PRIMARY_BASE} , 
        {IDE_PRIMARY_BASE , IDE_DEVICECONTROL_PRIMARY_BASE} , 
        {IDE_SECONDARY_BASE , IDE_DEVICECONTROL_SECONDARY_BASE} , 
        {IDE_SECONDARY_BASE , IDE_DEVICECONTROL_SECONDARY_BASE} , 
    };
    blockdev::block_device *devices[4]; // Four devices, Two primary, Two secondary

    debug::out::printf_function(DEBUG_INFO , "ide_cd_driver::prepare" , "Searching every ide cd drives...\n");
    for(int i = 0; i < 4; i++) {
        devices[i] = create_empty_device<blockdev::block_device>();
        designate_resources_count<blockdev::block_device>(devices[i] , 2 , 0 , 1 , 0);

        devices[i]->resources.io_ports[0] = io_port_info[i][0];
        devices[i]->resources.io_ports[1] = io_port_info[i][1];
        devices[i]->resources.flags[0] = is_master_info[i];

        if(blockdev::register_device(this , devices[i]) == INVALID) {
            debug::out::printf_function(DEBUG_WARNING , "ide_cd_driver::prepare" , "Device not found in idecd%d\n" , i);
        }
        else {
            debug::out::printf_function(DEBUG_SPECIAL , "ide_cd_driver::prepare" , "Registered device : idecd%d\n" , i);
        }
    }
    return true;
}

bool ide_cd_driver::send_command(word base_port , byte *command) {
    io_write_byte(base_port+IDE_PORT_COMMAND_IO , 0xA0);
    if(ide_driver::wait(base_port) == false) return false;

    for(int i = 0; i < 6; i++) {
        io_write_word(base_port+IDE_PORT_DATA , ((word *)command)[i]);
    }
    return true;
}

bool ide_cd_driver::open(blockdev::block_device *device) {
    return true;
}

bool ide_cd_driver::close(blockdev::block_device *device) {
    return true;
}

// We need to detect two device types, ATA HDD Drive and ATA CDROM 
bool ide_cd_driver::get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) {
    // Seperate to one getting CDROM, one getting HDD
    word status;
    word data[256];
    io_port base_port = device->resources.io_ports[0];
    io_port device_control_port = device->resources.io_ports[1];
    ide::cd_geometry_t cd_geometry;

    if(device->resources.io_port_count != 2) return false;
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    io_write_byte(base_port+IDE_PORT_COMMAND_IO , 0xA1); // IDENTIFY
    
    ide_driver::wait(base_port);
    for(int i = 0; i < 256; i++) {
        data[i] = io_read_word(base_port+IDE_PORT_DATA);
    }
    memcpy(&(cd_geometry) , data , 256);
    if((cd_geometry.config & 0x1F00) != 0x500) return false;
    /*
    char model[41];
    int i , j;
    for(i = 0 , j = 0; i < 20; i++) {
        model[j++] = data[27+i] >> 8;
        model[j++] = data[27+i] & 0xFF;
    }
    model[j++] = 0x00;
    debug::out::printf("model : %s\n" , model);
    */
    return get_cdrom_size(device , geometry);
}

bool ide_cd_driver::get_cdrom_size(blockdev::block_device *device , blockdev::device_geometry &geometry) {
    byte command[12] = {0x25 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
    io_port base_port = device->resources.io_ports[0];
    byte received_data[8] = {0 , };
    
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    io_write_byte(base_port+IDE_PORT_FEATURES , 0);
    io_write_byte(base_port+IDE_PORT_LBAMIDDLE , 0x08);
    io_write_byte(base_port+IDE_PORT_LBAHIGH , 0x08);
    send_command(base_port , command);
    if(ide_driver::wait(base_port) == false) return false;
    for(int i = 0; i < 4; i++) {
        ((word *)(&(received_data)))[i] = io_read_word(base_port+IDE_PORT_DATA);
    }
    geometry.is_chs = false;
    geometry.block_size = ((received_data[4] << 24)|(received_data[5] << 16)|(received_data[6] << 8)|received_data[7]);
    geometry.lba_total_block_count = ((received_data[0] << 24)|(received_data[1] << 16)|(received_data[2] << 8)|received_data[3]);
    geometry.lba_total_block_count += 1;
    debug::out::printf_function(DEBUG_TEXT , "ide_cd_drv::get_sz" , "Total sector count : %d\n" , geometry.lba_total_block_count);
    debug::out::printf_function(DEBUG_TEXT , "ide_cd_drv::get_sz" , "block size         : %d\n" , geometry.block_size);
    // ok good now you need to clean this code up
    return true;
}

max_t ide_cd_driver::read(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) {
    dword transfered_size;
    dword bytes_per_sector = 2048; // temporary!
    byte command[12] = {0xA8 , 0
                                      , (block_address >> 24) & 0xFF // read
                                      , (block_address >> 16) & 0xFF
                                      , (block_address >> 8) & 0xFF
                                      , block_address & 0xFF
                                      , (count >> 24) & 0xFF
                                      , (count >> 16) & 0xFF
                                      , (count >> 8) & 0xFF
                                      , count & 0xFF , 0x00 , 0x00};
    io_port base_port = device->resources.io_ports[0];
    if(device->resources.flags[0] == true) io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    else io_write_byte(base_port+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    
    if(ide_driver::wait(base_port) == false) return 0x00;

    io_write_byte(base_port+IDE_PORT_ERROR , 0x00);
    io_write_byte(base_port+IDE_PORT_LBAMIDDLE , (bytes_per_sector) & 0xFF);
    io_write_byte(base_port+IDE_PORT_LBAHIGH , (bytes_per_sector >> 8) & 0xFF);
    send_command(base_port , command);
    for(max_t i = 0; i < count; i++) {
        if(ide_driver::wait(base_port) == false) {
            return i*bytes_per_sector;
        }
        transfered_size = io_read_byte(base_port+IDE_PORT_LBAMIDDLE)|(io_read_byte(base_port+IDE_PORT_LBAHIGH) << 8);
        for(dword j = 0; j < transfered_size; j += 2) {
            *((word *)(((byte *)buffer)+((i*bytes_per_sector)+j))) = io_read_word(base_port+IDE_PORT_DATA);
        }
    }
    return count*bytes_per_sector;
}

max_t ide_cd_driver::write(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) {
    return 0;
}

bool ide_cd_driver::io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) {
    return false;
}

bool ide_cd_driver::io_write(blockdev::block_device *device , max_t command , max_t argument) {
    return false;
}

static void init_ide_driver(void) {
    io_write_byte(IDE_DEVICECONTROL_PRIMARY_BASE+IDE_PORT_DIGITAL_OUTPUT , 0);
    io_write_byte(IDE_DEVICECONTROL_SECONDARY_BASE+IDE_PORT_DIGITAL_OUTPUT , 0);
    interrupt::general::register_interrupt(32+14 , ide::interrupt_handler_irq14 , INTERRUPT_HANDLER_LEVEL_KERNEL|INTERRUPT_HANDLER_HARDWARE);
    interrupt::general::register_interrupt(32+15 , ide::interrupt_handler_irq15 , INTERRUPT_HANDLER_LEVEL_KERNEL|INTERRUPT_HANDLER_HARDWARE);
    blockdev::register_driver(new ide_driver , "idehd");
    blockdev::register_driver(new ide_cd_driver , "idecd");
}

REGISTER_DRIVER(init_ide_driver)