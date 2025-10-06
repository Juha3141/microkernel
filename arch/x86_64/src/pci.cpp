#include <kernel/driver/pci.hpp>
#include <x86_64/pci.hpp>
#include <kernel/io_port.hpp>

dword pci::read_config_space_dword(byte bus , byte device , byte function , byte offset) {
/* To-do : Support more than one configuration space access methods */
    dword config_addr = (offset & 0b11111100)
    |(((dword)function & 0b111) << 8)
    |(((dword)device & 0x1F) << 11)
    |(((dword)bus & 0xFF) << 16)
    |(1 << 31);

    io_write_dword(x86_64_PCI_CONFIG_ADDR , config_addr);
    return io_read_dword(x86_64_PCI_CONFIG_DATA);
}