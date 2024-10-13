#ifndef STRING_H
#define STRING_H

#include <stdint.h>

void *memset(void *addr, int c, uint64_t n);

void *memcpy(void *dest, void *src, uint64_t n);

void *memmove(void *dest, const void *src, uint64_t n);

int memcmp(const void *s1, const void *s2, uint64_t n);

uint64_t strlen(const char* str);

#endif