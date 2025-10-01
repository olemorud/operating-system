#pragma once

#include <limits.h>
#include <stddef.h>
#include "str.h"

__attribute__((noreturn))
void panic(struct str s);

/* bit_ceil returns the smallest power of 2 greater than `n` */
static inline uint32_t bit_ceil32(uint32_t n)
{
    /* __builtin_clz is undefined for n = 0 */
    return n
        ? 1 << (sizeof(n)*CHAR_BIT - __builtin_clz(n))
        : 1;
}

void* memmove(void *dest, const void *src, size_t n);
void* memset(void *s, int c, size_t n);
void* memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int printf(struct str format, ...);
