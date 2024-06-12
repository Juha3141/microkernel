#ifndef _IO_PORT_HPP_
#define _IO_PORT_HPP_

#include <kernel/essentials.hpp>

typedef unsigned short io_port;

void io_write_byte(io_port port , byte data);
byte io_read_byte(io_port port);

void io_write_word(io_port port , word data);
word io_read_word(io_port port);

void io_write_dword(io_port port , dword data);
dword io_read_dword(io_port port);

void io_write_qword(io_port port , qword data);
qword io_read_qword(io_port port);

#endif