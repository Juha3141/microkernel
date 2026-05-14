#include <string.hpp>
#include <kernel/mem/kmem_manager.hpp>

extern "C" void atexit() { return; }

String::String(const String& str_obj) {
    length   = str_obj.length;
    c_string = (char *)memory::pmem_alloc(length);
    strcpy(c_string , str_obj.c_string);
    capacity = str_obj.capacity;
}

String::String(size_t len) {
    c_string = (char *)memory::pmem_alloc(len+1);
    length = 0;
    capacity = len+1;
}

String::String(const char *str) {
    length = strlen(str)+1;
    c_string = (char *)memory::pmem_alloc(length);
    capacity = length;
    strcpy(c_string , str);
}

String::~String() { memory::pmem_free(c_string); }

void String::reserve(size_t sz) {
    if(capacity >= sz) return;
    memory::pmem_free(c_string);
    c_string = (char *)memory::pmem_alloc(sz);
    capacity = sz;
}

void String::clear() {
    length = 0;
    c_string[0] = '\0';
}

bool String::backspace() {
    if(length == 0) return false;
    
    c_string[length-1] = '\0';
    length--;
    return true;
}

String& String::operator+=(const String& str) {
    return operator+=(str.c_string);
}

String& String::operator+=(const char *str) {
    max_t new_len = length;
    max_t add_len = strlen(str);
    new_len += add_len;
    debug::out::printf("str to be added : %s\n" , str);
    debug::out::printf("new_len : %d\n" , new_len);
    // update the capacity
    if(new_len > capacity) {
        capacity = max(capacity*2+1 , new_len*2+1);
        char *prev_ptr = c_string;

        c_string = (char *)memory::pmem_alloc(capacity);
        strcpy(c_string , prev_ptr);
        memory::pmem_free(prev_ptr);
    }
    max_t i = 0;
    for(; i < add_len; i++) {
        c_string[i+length] = str[i];
    }
    c_string[i+length] = 0;
    length = new_len;
    return *this;
}

String& String::operator+=(const char& c) {
    if(length+1 > capacity) {
        if(capacity == 0) capacity = 1;
        capacity = (capacity*2) + 1;
        memory::pmem_free(c_string);
        c_string = (char *)memory::pmem_alloc(capacity);
    }
    c_string[length] = c;
    c_string[length+1] = '\0';
    length += 1;
    return *this;
}

String String::operator+(const String& rhs) {
    String copy_str = *this;
    copy_str += rhs;
    return copy_str;
}

String String::operator+(const char *str) {
    String copy_str = *this;
    copy_str += str;
    return copy_str;
}

String String::operator+(const char &c) {
    String copy_str = *this;
    copy_str += c;
    return copy_str;
}

String& String::operator=(const char *str) {
    length = strlen(str);
    memory::pmem_free(c_string);
    c_string = (char *)memory::pmem_alloc(length);
    capacity = length;
    strcpy(c_string , str);
    return *this;
}

String& String::operator=(const String &str) {
    length = str.length;
    memory::pmem_free(c_string);

    c_string = (char *)memory::pmem_alloc(length);
    capacity = str.capacity;
    strcpy(c_string , str.c_string);
    return *this;
}

String String::substr(max_t idx , max_t size) const {
    String new_str(size);
    strncpy(new_str.c_str() , this->c_string+idx , size);
    new_str.length = size;
    return new_str;
}

String String::substr(max_t idx) const {
    String new_str(this->length);
    strcpy(new_str.c_str() , this->c_string+idx);
    new_str.length = this->length-idx;
    return new_str;
}