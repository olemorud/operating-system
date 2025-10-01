#pragma once

#include <stddef.h>
#include <stdint.h>

struct str {
    const char*  data; 
    const size_t len;
};

#define str_attach(cstr) ((struct str){.data = cstr, .len = sizeof(cstr)-1})

/*
 * Returns a slice of `s`, `end`-exclusive
 * str_slice("hello", 1, 4) would return "ell"
 * */
static inline struct str str_slice(struct str s, size_t begin, size_t end)
{
    return (struct str) {
        .data = s.data + begin,
        .len = s.len - begin - end,
    };
}

/**
 * If a and b are not equal, str_compare returns the index where a and b
 * diverges. Otherwise returns -1
 */
bool str_equals(struct str a, struct str b);

/**
 * If a and b are equals, str_compare returns true, otherwise returns false
 */
int str_compare(struct str a, struct str b);

#define CSTR_(x) #x
#define CSTR(x) CSTR_(x)
