#include <kernel/mem/pages_manager.hpp>
#include <kernel/debug.hpp>

static max_t calculate_bitmap_area(max_t mem_start_address , max_t mem_end_address , max_t page_size) {
    max_t number_of_pages = (mem_end_address-mem_start_address)/page_size;
    max_t bitmap_area_size = (number_of_pages*PAGES_MANAGER_BITS_PER_PAGE)/8;
    
    debug::out::printf("bitmap_area_size : %dB\n" , bitmap_area_size);
    return bitmap_area_size;
}

void memory::PagesManager::init(max_t bitmap_start_address , max_t end_address , max_t page_size) {
    this->page_size = page_size;

    max_t bitmap_area_size = calculate_bitmap_area(bitmap_start_address , end_address , page_size);
    this->bitmap_start_address = bitmap_start_address;
    this->bitmap_end_address   = bitmap_start_address+bitmap_area_size;
    this->mem_start_address = bitmap_start_address+align_address(bitmap_area_size , page_size);
    this->mem_end_address = end_address;
}

max_t memory::PagesManager::allocate(max_t number_of_pages) {
    return 0x00;
}

bool memory::PagesManager::free(max_t ptr) {
    return false;
}