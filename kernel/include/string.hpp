#ifndef _ESSENTIAL_LIBRARY_HPP_
#define _ESSENTIAL_LIBRARY_HPP_

#include <stdarg.h>
#include <kernel/essentials.hpp>
#include <pair.hpp>

class String {
public:
    String() = default;
    String(const String& str_obj);
    String(size_t len);
    String(const char *str);

    size_t size() const;
    size_t max_size() const;
    bool empty() const;

    ///////// Change in capacity ///////// 
    void reserve(size_t sz);
    void clear();

    const char *c_str();

    char& operator[](int64_t idx);
    String& operator+=(const String& str);
    String& operator+=(const char *str);

    String& substr(int idx , int size);
    String& substr(int idx);
private:
    char *c_string;
    size_t length;
    size_t capacity;
};

extern "C" void *memset(void *dest , int c , size_t n);
extern "C" void *memcpy(void *dest , const void *src , size_t n);
extern "C" void *memmove(void *dest , const void *src , size_t n);
extern "C" int memcmp(const void *s1 , const void *s2 , size_t n);

extern "C" void *unsanitized_memset(void *dest , int c , size_t n);
extern "C" void *unsanitized_memcpy(void *dest , const void *src , size_t n);
extern "C" void *unsanitized_memmove(void *dest , const void *src , size_t n);
extern "C" int unsanitized_memcmp(const void *s1 , const void *s2 , size_t n);

size_t strlen(const char *s);
char *strcpy(char *dest , const char *src);
char *strncpy(char *dest , const char *src , size_t n);
char *strcat(char *dest , const char *src);
char *strncat(char *dest , const char *src , size_t n);
int strcmp(const char *s1 , const char *s2);
int strncmp(const char *s1 , const char *s2 , size_t n);

Pair<int , bool>vsprintf(char *buf , const char *fmt , va_list ap , char contains);
int vsprintf(char *buf , const char *fmt , va_list ap);
int sprintf(char *buf , const char *fmt , ...);

int atoi(const char *nptr);
long int atol(const char *nptr);
long long int atoll(const char *nptr);
char *itoa(int value , char *result , int base);
template <typename T> extern char *itoa_variation(T value , char *result , int base , bool lowercase=true);


#endif