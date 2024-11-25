OUTPUT_FORMAT(elf32-i386)
ENTRY(entry)

SECTIONS {
  . = 0x4000;
  .entry 0x4000 : {
    *(.entry .entry.*)
  }
  .text ALIGN(512) : {
    *(.text .stub .text.* .gnu.linkonce.t.*)
  }
  .data ALIGN(512) : {
    *(.data .data.* .gnu.linkonce.d.*)
  }
  .rodata ALIGN(512) : {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }
  .rodata1 ALIGN(512) : {
    *(.rodata1)
  }
  .bss ALIGN(512) : {
    *(.bss .bss.* .gnu.linkonce.b.*)
  }
}