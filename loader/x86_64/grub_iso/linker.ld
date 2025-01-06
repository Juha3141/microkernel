OUTPUT_FORMAT(elf32-i386)
ENTRY(Entry)
SECTIONS {
  . = 0x100000;
  .multiboot_header ALIGN(4) : {
    *(.multiboot_header .multiboot_header.*)
  }
  .text : {
    *(.text .stub .text.* .gnu.linkonce.t.*)
  }
  .data : {
    *(.data .data.* .gnu.linkonce.d.*)
  }
  .rodata : {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }
  .rodata1 : {
    *(.rodata1)
  }
  .bss : {
    *(.bss .bss.* .gnu.linkonce.b.*)
  }
}