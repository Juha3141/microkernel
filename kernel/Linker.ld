kernel_linear_address = 0xC0000000;

SECTIONS {
  . = kernel_linear_address;
  .text : AT(kernel_linear_address) {
    *(.text) *(.text.*)
  }
  .data ALIGN(4096) : {
    *(.data)
  }
  .bss ALIGN(4096) : {
    *(.bss) *(.rodata) *(.bss.*) *(COMMON)
  }
}