#include <kernel/io_port.hpp>

void io_write_byte(io_port port , byte data) {
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("mov al , %0"::"r"(data));
    __asm__ ("out dx , al");
}

byte io_read_byte(io_port port) {
    byte data;
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("in al , dx");
    __asm__ ("mov %0 , al":"=r"(data));
    return data;
}


void io_write_word(io_port port , word data) {
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("mov ax , %0"::"r"(data));
    __asm__ ("out dx , ax");
}

word io_read_word(io_port port) {
    word data;
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("in ax , dx");
    __asm__ ("mov %0 , ax":"=r"(data));
    return data;
}


void io_write_dword(io_port port , dword data) {
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("mov eax , %0"::"r"(data));
    __asm__ ("out dx , eax");
}

dword io_read_dword(io_port port) {
    dword data;
    __asm__ ("mov dx , %0"::"r"(port));
    __asm__ ("in eax , dx");
    __asm__ ("mov %0 , eax":"=r"(data));
    return data;
}