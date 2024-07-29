#pragma once

#include <stddef.h>

struct str {
    char* data; 
    size_t len;
};

#define str_attach(cstr) (struct str){.data = cstr, .len = sizeof(cstr)-1}
