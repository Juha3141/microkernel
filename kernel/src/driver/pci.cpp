#include <kernel/driver/pci.hpp>
#include <kernel/debug.hpp>

static void get_common_header_field(struct pci::common_header_field &header , byte bus , byte device , byte function) {
    dword *header_ptr = (dword *)(&header);

    for(byte i = 0, j = 0; i < sizeof(struct pci::common_header_field); i += 4) {
        header_ptr[j++] = pci::read_config_space_dword(bus , device , function , i);
    }
}

static word get_vendor_id(byte bus , byte device , byte function) {
    dword first_dword = pci::read_config_space_dword(bus , device , function , 0);
    return (word)(first_dword & 0xffff);
}

static void probe_function(byte bus , byte device , byte function) {
    struct pci::common_header_field header;
    get_common_header_field(header , bus , device , function);
    if(header.vendor_id == 0xffff) return;

    debug::out::printf("bus %d, device %d, function %d  -->  " , bus , device , function);
    debug::out::raw_printf("vendor_id = 0x%04X, device_id = 0x%04X\n" , header.vendor_id , header.device_id);

    // how to register the device to the 
    
}

static void probe_device(byte bus , byte device) {
    struct pci::common_header_field header;
    
    get_common_header_field(header , bus , device , 0);
    if(header.vendor_id == 0xffff) return; // the device does not exist
    
    probe_function(bus , device , 0);
    if((header.header_type & 0x80) == 0x80) { // multi-function device
        for(byte function = 1; function < 8; function++) {
            probe_function(bus , device , function);
        }
    }
}

static void probe_bus(byte bus) {
    for(byte device = 0; device < 32; device++) {
        probe_device(bus , device);
    }
}

#include <kernel/debug.hpp>

void pci::probe_all_pci_devices(void) {
    // brute force way of probing all the pci devices
    for(int bus = 0; bus < 256; bus++) {
        probe_bus(bus);
    }
}

