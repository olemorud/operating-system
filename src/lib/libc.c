/*
 *  This file is for any library function that require no system calls. In
 *  other words it's for freestanding functions.
 *  */

#include <stddef.h>
#include <stdint.h>
#include "tty.h"
#include "str.h"

/*
 * Error handling
 * ==============
 * */
__attribute__((noreturn))
void panic(struct str s)
{
    terminal_clear();
    terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    terminal_write(s);
    __asm__ volatile("hlt");
    __builtin_unreachable();
}

/* 
 * Stack smashing protection
 * =========================
 * */
uintptr_t __stack_chk_guard = 0x00112233; // this is just a random value

__attribute__((noreturn))
void __stack_chk_fail(void)
{
    panic(str_attach("Stack smashing detected"));
}

/* 
 * State-enforced suffering 
 * ========================
 * GCC and Clang reserve the right to generate calls to the following
 * 4 functions even if they are not directly called.
 * Implement them as the C specification mandates.
 * DO NOT remove or rename these functions, or stuff will eventually break!
 * They CAN be moved to a different .c file.
 * */
void* memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void* memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void* memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

