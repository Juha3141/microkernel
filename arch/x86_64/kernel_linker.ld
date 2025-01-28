ENTRY(kernel_main)

INCLUDE arch_configurations.ld

SECTIONS {
  /* temporarily change the kernel load address for debugging purpose */
  . = 0x100000;
  .entry 0x100000 : {
    *(.entry) *(.entry.*)
  }
  .text ALIGN(4096) : {
    *(.text) *(.text.*)
  }
  .debug_interface ALIGN(4096) : {
    __debug_interface_start__ = .;
    *(.debug_interface .debug_interface.*) 
    __debug_interface_end__ = .;
  }
  .drivers_init ALIGN(4096) : {
    __drivers_init_start__ = .; /* each different levels */
    *(.drivers_init.0 .drivers_init.0.*) 
    *(.drivers_init.1 .drivers_init.1.*) 
    *(.drivers_init.2 .drivers_init.2.*) 
    *(.drivers_init.3 .drivers_init.3.*) 
    *(.drivers_init.4 .drivers_init.4.*) 
    *(.drivers_init.5 .drivers_init.5.*) 
    *(.drivers_init.6 .drivers_init.6.*) 
    *(.drivers_init.7 .drivers_init.7.*) 
    *(.drivers_init.8 .drivers_init.8.*) 
    *(.drivers_init.9 .drivers_init.9.*) 
    *(.drivers_init.10 .drivers_init.10.*) 
    *(.drivers_init.11 .drivers_init.11.*) 
    *(.drivers_init.12 .drivers_init.12.*) 
    *(.drivers_init.13 .drivers_init.13.*) 
    *(.drivers_init.14 .drivers_init.14.*) 
    *(.drivers_init.15 .drivers_init.15.*) 
    __drivers_init_end__ = .;
  }
  .data ALIGN(4096) : {
    __data_start__ = .;
    *(.data)
    __data_end__ = .;
  }
  .bss ALIGN(4096) : {
    __bss_start__ = .;
    *(.bss) *(.rodata) *(.bss.*) *(COMMON)
    __bss_end__ = .;
  }
}