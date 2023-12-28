#ifndef _ESSENTIAL_LIBRARY_HPP_
#define _ESSENTIAL_LIBRARY_HPP_

#include <stdarg.h>
#include <interface_type.hpp>

#define MIN(X , Y) ((X) >= (Y) ? (Y) : (X))
#define MAX(X , Y) ((X) >= (Y) ? (X) : (Y))

extern void *memset(void *s , int c , size_t n);
extern void *memcpy(void *dest , const void *src , size_t n);
extern int memcmp(const void *s1 , const void *s2 , size_t n);

extern size_t strlen(const char *s);
extern char *strcpy(char *dest , const char *src);
extern char *strncpy(char *dest , const char *src , size_t n);
extern char *strcat(char *dest , char *src);
extern char *strncat(char *dest , const char *src , size_t n);
extern int strcmp(const char *s1 , const char *s2);
extern int strncmp(const char *s1 , const char *s2 , size_t n);

extern int vsprintf(char *buf , const char *fmt , va_list ap);
extern int sprintf(char *buf , const char *fmt , ...);

extern int atoi(const char *nptr);
extern long int atol(const char *nptr);
extern long long int atoll(const char *nptr);
extern char *itoa(int value , char *result , int base);
template <typename T> extern char *itoa_variation(T value , char *result , int base);


#endif