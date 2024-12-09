ENTRY(kernel_main)

INCLUDE arch_configurations.ld

SECTIONS {
  . = _KERNEL_LOAD_LOCATION;
  .entry _KERNEL_LOAD_LOCATION : {
    *(.entry) *(.entry.*)
  }
  .text ALIGN(4096) : {
    *(.text) *(.text.*)
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