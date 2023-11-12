/**
 * @file interface.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief Basic interfaces for kernel systems
 * @date 2023-11-12
 */

#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <interface_type.hpp>

namespace interface {
    // I/O port
    io_data IO_read(io_port port);
    io_data IO_write(io_port port);

    // interrupt
    void initialize_interrupt();
    void register_interrupt(dword int_number , interrupt_handler handler);
    void deregister_interrupt(dword int_number);

    // timer system
    void initialize_timer_system(interrupt_handler Handler);

    // segmentation
    bool supportation_segment();

    void initialize_segment();
    void register_segment();
    void deregister_segment();

    // memory model & memory model
    byte system_memory_model(void); // work just as like a constant
    int get_memory_map(memory_map *memmap);

    // paging
    bool supportation_paging();
    
    void initialize_paging();
    void set_page_entry(); // level, page number, properties ...

    // CPU instructions
    void pause(void);
    void halt(void);
    qword rdtsc(void);
    
    // Multiprocessor Specified
    bool supportation_mp();
    
    // CPU information
    bool get_general_cpu_info(general_cpu_info &cpuinfo);
};

#endif