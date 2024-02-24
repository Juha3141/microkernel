#ifndef _IDE_HPP_
#define _IDE_HPP_

#include <block_device_driver.hpp>
#include <storage_system.hpp>

#define IDE_PRIMARY_BASE                 0x1F0
#define IDE_SECONDARY_BASE               0x170
#define IDE_DEVICECONTROL_PRIMARY_BASE   0x3F0
#define IDE_DEVICECONTROL_SECONDARY_BASE 0x370

#define IDE_PORT_DATA           0x00
#define IDE_PORT_ERROR          0x01
#define IDE_PORT_FEATURES       0x01
#define IDE_PORT_SECTOR_COUNT   0x02
#define IDE_PORT_LBALOW         0x03
#define IDE_PORT_LBAMIDDLE      0x04
#define IDE_PORT_LBAHIGH        0x05
#define IDE_PORT_DRIVE_SELECT   0x06
#define IDE_PORT_COMMAND_IO     0x07

#define IDE_PORT_DIGITAL_OUTPUT 0x06
#define IDE_PORT_DRIVE_ADDRESS  0x07

#define IDE_STATUS_ERROR           0b00000001
#define IDE_STATUS_INDEX           0b00000010
#define IDE_STATUS_CORRECT_DATA    0b00000100
#define IDE_STATUS_DATA_REQUEST    0b00001000
#define IDE_STATUS_SEEK_DONE       0b00010000
#define IDE_STATUS_WRITE_FAULT     0b00100000
#define IDE_STATUS_READY           0b01000000
#define IDE_STATUS_BUSY            0b10000000

#define IDE_DRIVER_FLAG_LBA        0b00000100
#define IDE_DRIVER_FLAG_SLAVE      0b00010000

#define IDE_DIGITAL_OUTPUT_INTERRUPT_ENABLE 0b010
#define IDE_DIGITAL_OUTPUT_SOFTWARE_RESET   0b100

#define IDE_STATUS_ERROR 0b00000001
#define IDE_STATUS_INDEX 0b00000010
#define IDE_STATUS_CORR  0b00000100
#define IDE_STATUS_DRQ   0b00001000
#define IDE_STATUS_SRV   0b00010000
#define IDE_STATUS_DF    0b00100000
#define IDE_STATUS_READY 0b01000000
#define IDE_STATUS_BUSY  0b10000000

// ide --- io_read & io_write
#define IDE_IO_READ_GET_MODEL 0x01


// ide_cd --- io_read & io_write
#define IDE_CD_IO_READ_GET_MODEL 0x01

struct ide_driver : blockdev::block_device_driver {
    static void register_driver(void);

    bool prepare(void) override;
    max_t read(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) override;
    max_t write(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) override;
    bool get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) override;
    bool io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(blockdev::block_device *device , max_t command , max_t argument) override;

    static bool wait(io_port base_port);
    static bool primary_interrupt_flag;
    static bool secondary_interrupt_flag;
};

struct ide_cd_driver : blockdev::block_device_driver { // inherit from IDEDriver or StorageDriver??
    static void register_driver(void);
    
    bool prepare(void) override;
    max_t read(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) override;
    max_t write(blockdev::block_device *device , max_t block_address , max_t count , void *buffer) override;
    bool get_geometry(blockdev::block_device *device , blockdev::device_geometry &geometry) override;
    bool io_read(blockdev::block_device *device , max_t command , max_t argument , max_t &data_out) override;
    bool io_write(blockdev::block_device *device , max_t command , max_t argument) override;

    static bool send_command(io_port base_port , byte *command);
    static bool get_cdrom_size(blockdev::block_device *device , blockdev::device_geometry &geometry);
};

namespace ide {
    typedef struct geometry_s {
        word config;
        word reserved_1[9];
        word serial[10];
        word reserved_2[3];
        word firmware[4];
        word model[20];
        word reserved_3[13];
        dword total_sectors;
        word reserved_4[196];
    }geometry_t;
    typedef struct cd_geometry_s {
        word config;
        word reserved_1[9];
        word serial[10];
        word reserved_2[3];
        word firmware[4];
        word model[20];
        word reserved_3[210];
    }cd_geometry_t;
    void interrupt_handler_irq14(void);
    void interrupt_handler_irq15(void);
    
    void main_int_handler(bool Primary);
}

#endif