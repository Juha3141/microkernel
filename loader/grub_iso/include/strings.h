#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <stdarg.h>

#define size_t unsigned int

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

#endif