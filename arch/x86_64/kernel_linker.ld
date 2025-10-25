ENTRY(kernel_main)

INCLUDE arch_configurations.ld

SECTIONS {
  /* temporarily change the kernel load address for debugging purpose */
  . = 0x100000;
  .entry 0x100000 : {
    *(.entry) *(.entry.*)
  }
  .text ALIGN(4096) : {
    __text_start__ = .;
    *(.text) *(.text.*)
    __text_end__ = .;
  }
  .debug_interface ALIGN(4096) : {
    __debug_interface_start__ = .;
    *(.debug_interface .debug_interface.*)
    __debug_interface_end__ = .;
  }
  .drivers_init ALIGN(8) : {
    __drivers_init_start__ = .; /* each different levels */
    *(.drivers_init) *(.drivers_init.*) /* ignore the priority for now */
    __drivers_init_end__ = .;
  }
  .fs_drivers_init ALIGN(8) : {
    __fs_drivers_init_start__ = .;
    *(.fs_drivers_init) *(.fs_drivers_init.fs.*)
    __fs_drivers_init_end__ = .;
  }
  .data ALIGN(4096) : {
    __data_start__ = .;
    *(.data)
    __data_end__ = .;
  }
  .bss ALIGN(4096) : {
    __bss_start__ = .;
    *(.bss) *(.rodata) *(.bss.*)
    __bss_end__ = .;
  }
  .singleton_obj_ptrs ALIGN(4096) : {
    __singleton_obj_ptrs_start__ = .;
    *(.singleton_obj_ptrs) *(.singleton_obj_ptrs.*)
    __singleton_obj_ptrs_end__ = .;
  }
  .gnu.linkonce ALIGN(4096) : {
    __gnu_linkonce_start__ = .;
    *(.gnu.linkonce.*) 
    __gnu_linkonce_end__   = .;
  }
}