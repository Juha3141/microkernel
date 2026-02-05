#ifndef _PCI_HPP_
#define _PCI_HPP_

#include <kernel/types.hpp>
#include <kernel/mem/kmem_manager.hpp>

namespace pci {
    dword ARCHDEP read_config_space_dword(byte bus , byte device , byte function , byte offset);
    void probe_all_pci_devices(void);

    bool read_device_configuration_space(max_t vendor_id , max_t device_id , void *configuration_space);
    bool write_device_configuration_dword(max_t vendor_id , max_t device_id , byte offset);

    struct PCIDeviceContainer {
        SINGLETON_PATTERN_PMEM(PCIDeviceContainer);

        void init(void);
        void register_device(byte bus , byte device , byte function , max_t vendor_id , max_t device_id);
    };

    struct common_header_field {
        uint16_t vendor_id;
        uint16_t device_id;
    
        uint16_t command;
        uint16_t status;
    
        uint8_t revision_id;
        uint8_t prog_if; /* programming interface byte */
        uint8_t subclass;
        uint8_t class_code;
        uint8_t cache_line_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t BIST; /* built-in self test*/
    };
    // General devices
    struct header_type_0x00 {
        struct common_header_field header;

        uint32_t BAR[6]; /* Base Address Registers */
        uint32_t cardbus_CIS_ptr;
        uint16_t subsystem_vendor_id;
        uint16_t subsystem_id;
        uint32_t expansion_ROM_base_addr;
        uint8_t capabilities_ptr;

        uint8_t reserved[7];

        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint8_t min_grant;
        uint8_t max_latency;
    };
    // PCI-to-PCI bridges
    struct header_type_0x01 {
        struct common_header_field header;
        
        uint32_t BAR[2]; /* Base Address Registers */
        uint8_t primary_bus_num;
        uint8_t secoundary_bus_num;
        uint8_t subordinate_bus_num;
        uint8_t secondary_latency_timer;
        uint8_t io_base;
        uint8_t io_limit;
        uint16_t memory_limit;
        uint8_t prefetchable_memory_base;
        uint8_t prefetchable_memory_limit;
        uint32_t prefetchable_base_upper_32b;
        uint32_t prefetchable_limit_upper_32b;
        uint16_t io_base_upper_16b;
        uint16_t io_limit_upper_16b;

        uint8_t capabilities_ptr;
        uint8_t reserved[3];

        uint32_t expansion_rom_base_addr;
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint16_t bridge_control;
    };
    struct header_type_0x02 {
        struct common_header_field header;
        
        uint32_t cardbus_socket_base_addr;
        uint8_t capabilities_list_offset;
        uint8_t reserved_1;
        uint16_t secondary_status;
        uint8_t pci_bus_num;
        uint8_t cardbus_bus_num;
        uint8_t subordinate_bus_num;
        uint8_t cardbus_latency_timer;
        uint32_t memory_base_addr0;
        uint32_t memory_limit0;
        uint32_t memory_base_addr1;
        uint32_t memory_limit1;
        
        uint32_t io_base_addr0;
        uint32_t io_limit0;
        uint32_t io_base_addr1;
        uint32_t io_limit1;
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint16_t bridge_control;
        uint16_t subsystem_device_id;
        uint16_t subsystem_vendor_id;
        uint32_t pc_card_legacymode_base_addr;
    };
};

struct pci_device_identifier {
    max_t vendor_id;
    max_t device_id;
    
    bool is_multi_function;
    max_t function;
};

#endif