// klibc.h
#ifndef KLIBC_H
#define KLIBC_H

#include <stddef.h>

// Khai báo các hàm
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif // KLIBC_H
