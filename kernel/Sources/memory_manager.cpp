/**
 * @file memory_management.cpp
 * @brief Core system of memory management
 * 
 * @author Ian Juha Cho
 * contact : ianisnumber2027@gmail.com
 */

#include <kmem_manager.hpp>
#include <nodes_manager.hpp>
#include <string.hpp>
#include <debug.hpp>

void memory::init(void) {
	word memmap_entry_count = 0;
	max_t total_memory = 0;
	memory_map memmap[64];
	// memmap_entry_count = get_memory_map(memmap);
}
