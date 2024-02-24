kernel_linear_address = 0xC0000000;
ENTRY(kernel_main)

SECTIONS {
  . = kernel_linear_address;
  .text : {
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