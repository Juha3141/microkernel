#include <kernel/mem/pages_allocator.hpp>
#include <kernel/debug.hpp>

static max_t calculate_bitmap_area(max_t mem_start_address , max_t mem_end_address , max_t page_size) {
    max_t number_of_pages = (mem_end_address-mem_start_address)/page_size;
    max_t bitmap_area_size = (number_of_pages*PAGES_MANAGER_BITS_PER_PAGE)/8;
    
    // printf("bitmap_area_size : %lldB\n" , bitmap_area_size);
    return bitmap_area_size;
}

void memory::PagesManager::init(max_t bitmap_start_address , max_t end_address , max_t page_size) {
    this->page_size = page_size;

    max_t bitmap_area_size = calculate_bitmap_area(bitmap_start_address , end_address , page_size);
    this->bitmap_start_address = bitmap_start_address;
    this->bitmap_end_address   = bitmap_start_address+bitmap_area_size;
    this->mem_start_address = bitmap_start_address+alignto(bitmap_area_size , page_size);
    this->mem_end_address = end_address;

    this->memory_usage = 0;

    debug::out::printf("mem_start_address : 0x%llx\n" , this->mem_start_address);
    debug::out::printf("bitmap_size : 0x%llx\n" , bitmap_area_size);
}

inline void decrement_bitmap_offset(max_t &page_loc_word , max_t &page_loc_bit , max_t dec) {
    const max_t num_of_pages_per_word = (WORD_SIZE*8)/PAGES_MANAGER_BITS_PER_PAGE;

    if(page_loc_bit >= dec) {
        page_loc_bit -= dec;
        return;
    }
    page_loc_word -= 1;
    max_t dec_word = dec/num_of_pages_per_word;
    max_t dec_bit  = dec%num_of_pages_per_word;
    page_loc_word -= dec_word;
    page_loc_bit  += (num_of_pages_per_word-dec_bit);
}

inline void increment_bitmap_offset(max_t &page_loc_word , max_t &page_loc_bit , max_t inc) {
    const max_t num_of_pages_per_word = (WORD_SIZE*8)/PAGES_MANAGER_BITS_PER_PAGE;

    if(page_loc_bit+inc < num_of_pages_per_word) {
        page_loc_bit += inc;
        return;
    }
    page_loc_word += 1;
    max_t inc_word = inc/num_of_pages_per_word;
    max_t inc_bit  = inc%num_of_pages_per_word;
    page_loc_word += inc_word;
    page_loc_bit = (inc+page_loc_bit)%num_of_pages_per_word;
}

/// @brief Blocks the pages from the given start page number
/// @param start_page_number page number of the start page
/// @param page_count_to_block number of pages to block from the start_page_number
/// @return True if all the areas are blocked, False if part of the given area is occupied
bool memory::PagesManager::block_pages(max_t start_page_number , max_t page_count_to_block) {
    max_t *bitmap = (max_t *)this->bitmap_start_address;

    const max_t num_of_pages_per_word = (WORD_SIZE*8)/PAGES_MANAGER_BITS_PER_PAGE;
    max_t page_loc_word = start_page_number/num_of_pages_per_word , page_loc_bit = start_page_number%num_of_pages_per_word;
    for(int i = 0; i < page_count_to_block; i++) {
        char identifier = (bitmap[page_loc_word] >> page_loc_bit*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
        if(identifier != 0x00) { // The target area is occupied!
            return false;
        }
        increment_bitmap_offset(page_loc_word , page_loc_bit , 1);
    }
    page_loc_word = start_page_number/num_of_pages_per_word;
    page_loc_bit = start_page_number%num_of_pages_per_word;
    for(int i = 0; i < page_count_to_block; i++) {
        bitmap[page_loc_word] |= ((max_t)PAGES_MANAGER_IDENTIFIER_BLOCK << (page_loc_bit*PAGES_MANAGER_BITS_PER_PAGE));
        increment_bitmap_offset(page_loc_word , page_loc_bit , 1);
    }
    return true;
}

bool memory::PagesManager::unblock_pages(max_t start_page_number , max_t blocked_page_count) {
    return this->free(this->mem_start_address+(start_page_number*this->page_size) , true , blocked_page_count);
}

max_t memory::PagesManager::allocate(max_t number_of_pages) {
    max_t *bitmap = (max_t *)this->bitmap_start_address;
    max_t f_streak = 0;

    max_t page_loc_bit , page_loc_word;

    if((number_of_pages*this->page_size)+this->memory_usage >= (this->mem_end_address-this->mem_start_address)) {
        return 0x00;
    }

    bool found = false;
    const max_t num_of_pages_per_word = (WORD_SIZE*8)/PAGES_MANAGER_BITS_PER_PAGE;
    for(max_t i = 0; i < (this->bitmap_end_address-this->bitmap_start_address)/WORD_SIZE; i += 1) {
        max_t word = bitmap[i];
        for(max_t j = 0; j < num_of_pages_per_word; j++) {
            char b = (word >> j*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
#ifdef DEBUG
            debug::out::printf("word #% 3lld, bit #% 3lld : %d%d\n" , i , j , (b >> 1) & 0b01 , b & 0b01);
#endif
            if(b == 0) {
                f_streak++;
                if(f_streak >= number_of_pages) {
#ifdef DEBUG
                    debug::out::printf("found! location : #%lld word, #%lldth bit, f_streak=%lld\n" , i , j , f_streak);
#endif
                    page_loc_word = i;
                    page_loc_bit = j;
                    decrement_bitmap_offset(page_loc_word , page_loc_bit , f_streak-1);
#ifdef DEBUG
                    debug::out::printf("start location : #%lld word, #%lldth bit\n" , page_loc_word , page_loc_bit);
#endif

                    found = true;
                    break;
                }
            }
            else f_streak = 0;
        }
        if(found) break;
    }
    if(!found) return 0x00; // no available memory left!
    
    max_t page_location = (page_loc_word*num_of_pages_per_word)+page_loc_bit;
    debug::out::printf("actual page location : %d\n" , page_location);

    char prev_identifier = -1;
    int bit = page_loc_bit , word = page_loc_word;
    do {
        bit--;
        if(bit < 0) { word--; bit = num_of_pages_per_word-1; }
        char b = (bitmap[word] >> bit*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
#ifdef DEBUG
        debug::out::printf("(backtracing) word #% 3d, bit #% 3d : %d%d\n" , word , bit , (b >> 1) & 0b01 , b & 0b01);
#endif

        if(b != 0) { prev_identifier = b; break; }
    }while(word >= 0);
    if(prev_identifier == -1) {
        // search it forward
#ifdef DEBUG
        debug::out::printf("forward..\n");
#endif
        max_t bit = page_loc_bit , word = page_loc_word;
        increment_bitmap_offset(word , bit , number_of_pages);
        do {
            bit++;
            if(bit >= num_of_pages_per_word) { word++; bit = 0; }

            char b = (bitmap[word] >> bit*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
#ifdef DEBUG
            debug::out::printf("(forward tracing) word #% 3lld, bit #% 3lld : %d%d\n" , word , bit , (b >> 1) & 0b01 , b & 0b01);
#endif

            if(b != 0) { prev_identifier = b; break; }
        }while(word <= this->bitmap_end_address-this->bitmap_start_address);
    }

    debug::out::printf("prev_identifier = %d%d\n" , (prev_identifier >> 1) & 0b01 , prev_identifier & 0b01);
    char new_identifier = (prev_identifier == PAGES_MANAGER_BITS_MASK) ? 0b01 : ((prev_identifier+1) & PAGES_MANAGER_BITS_MASK);
    debug::out::printf("new_identifier  = %d%d\n" , (new_identifier >> 1) & 0b01 , new_identifier & 0b01);
    
    max_t k = 0;
    for(max_t i = page_loc_word;; i += 1) {
        bool quit = false;
        for(max_t j = ((i == page_loc_word) ? page_loc_bit : 0); j < num_of_pages_per_word; j++) {
            if(k >= number_of_pages) { quit = true; break; }
            k++;

            bitmap[i] |= ((max_t)new_identifier << (j*PAGES_MANAGER_BITS_PER_PAGE));

            char b = (bitmap[i] >> j*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
#ifdef DEBUG
            debug::out::printf("(write) word #% 3lld, bit #% 3lld : %d%d\n" , i , j , (b >> 1) & 0b01 , b & 0b01);
#endif
        }
        if(quit) break;
    }
    debug::out::printf("allocated page number : %ld\n" , page_location);
    this->memory_usage += number_of_pages*this->page_size;
    return (this->mem_start_address+(page_location*this->page_size));
}

bool memory::PagesManager::free(max_t ptr , bool free_blocked_area , max_t blocked_page_count) {
    max_t *bitmap = (max_t *)this->bitmap_start_address;
    max_t page_location = ((ptr-this->mem_start_address)/this->page_size);
    const max_t num_of_pages_per_word = (WORD_SIZE*8)/PAGES_MANAGER_BITS_PER_PAGE;
    if(!(ptr >= this->mem_start_address && ptr <= this->mem_end_address)) return false;
    
    max_t page_loc_word = page_location/num_of_pages_per_word , page_loc_bit = page_location%num_of_pages_per_word;
#ifdef DEBUG
    debug::out::printf("page_location : %d\n" , page_location);
#endif

    max_t prev_word = page_loc_word , prev_bit = page_loc_bit;
    char current_identifier = (bitmap[page_loc_word] >> page_loc_bit*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
    // free_blocked_area should be "true" in order to free the blocked area
    if(free_blocked_area == false && current_identifier == PAGES_MANAGER_IDENTIFIER_BLOCK) {
        return false;
    }
    decrement_bitmap_offset(prev_word , prev_bit , 1);
    char prev_identifier    = (bitmap[prev_word] >> prev_bit*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
#ifdef DEBUG
    debug::out::printf("current_identifier : %d\n" , current_identifier);
    debug::out::printf("prev_identifier    : %d\n" , prev_identifier);
#endif
    if(free_blocked_area == false && current_identifier == prev_identifier) {
        debug::out::printf("ptr not the start of the segment!\n");
        return false;
    }
    max_t size = 0;
    for(max_t i = page_loc_word;; i += 1) {
        bool quit = false;
        for(max_t j = ((i == page_loc_word) ? page_loc_bit : 0); j < num_of_pages_per_word; j++) {
            char b = (bitmap[i] >> j*PAGES_MANAGER_BITS_PER_PAGE) & PAGES_MANAGER_BITS_MASK;
            if(b != current_identifier) {quit = true; break; }

            bitmap[i] &= ~((max_t)PAGES_MANAGER_BITS_MASK << (j*PAGES_MANAGER_BITS_PER_PAGE));
            size++;
            if(free_blocked_area == true && blocked_page_count != 0 && size >= blocked_page_count) { quit = true; break; }
        }
        if(quit) break;
    }
    this->memory_usage -= size*this->page_size;
    debug::out::printf("freed memory size = %d\n" , size);
    return true;
}