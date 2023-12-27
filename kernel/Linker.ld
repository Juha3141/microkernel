OUTPUT_FORMAT(elf64-x86-64)
ENTRY(init_temp_stack)
SECTIONS {
  .text 0x103000 : {
    *(.text .text.*)
  }
  .data : {
    *(.data .data.*)
  }
  .bss : {
    *(.bss .bss.*)
  }
  .rodata : {
    *(.rodata .rodata.*)
  }
  .rodata1 : {
    *(.rodata1)
  }
}
