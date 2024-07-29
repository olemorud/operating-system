#pragma once

#include <stddef.h>
#include "str.h"
#include "tty.h"

__attribute__((noreturn)) void panic(struct str s);

void* memmove(void *dest, const void *src, size_t n);
void* memset(void *s, int c, size_t n);
void* memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int printf(struct str format, ...);
