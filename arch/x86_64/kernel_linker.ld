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
    *(.drivers_init) *(.drivers_init.*) /* ignore the priority for now */
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