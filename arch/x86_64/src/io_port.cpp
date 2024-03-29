#include <kernel/io_port.hpp>
#include <arch_inline_asm.hpp>

void io_write_byte(io_port port , byte data) {
    IA ("mov dx , %0"::"r"(port));
    IA ("mov al , %0"::"r"(data));
    IA ("out dx , al");
}

byte io_read_byte(io_port port) {
    byte data;
    IA ("mov dx , %0"::"r"(port));
    IA ("in al , dx");
    IA ("mov %0 , al":"=r"(data));
    return data;
}


void io_write_word(io_port port , word data) {
    IA ("mov dx , %0"::"r"(port));
    IA ("mov ax , %0"::"r"(data));
    IA ("out dx , ax");
}

word io_read_word(io_port port) {
    word data;
    IA ("mov dx , %0"::"r"(port));
    IA ("in ax , dx");
    IA ("mov %0 , ax":"=r"(data));
    return data;
}


void io_write_dword(io_port port , dword data) {
    IA ("mov dx , %0"::"r"(port));
    IA ("mov eax , %0"::"r"(data));
    IA ("out dx , eax");
}

dword io_read_dword(io_port port) {
    dword data;
    IA ("mov dx , %0"::"r"(port));
    IA ("in eax , dx");
    IA ("mov %0 , eax":"=r"(data));
    return data;
}