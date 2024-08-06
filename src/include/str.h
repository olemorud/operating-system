#pragma once

#include <stddef.h>

struct str {
    const char*  data; 
    const size_t len;
};

#define str_attach(cstr) (struct str){.data = cstr, .len = sizeof(cstr)-1}

static inline struct str str_slice(struct str s, size_t begin, size_t end)
{
    return (struct str) {
        .data = s.data + begin,
        .len = s.len - begin - end,
    };
}

#define CSTR_(x) #x
#define CSTR(x) CSTR_(x)
