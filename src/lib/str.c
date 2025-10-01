
#include "str.h"
#include <stdint.h>

int str_compare(struct str a, struct str b)
{
    const size_t min_len = a.len > b.len
        ? b.len
        : a.len;
    for (size_t i = 0; i < min_len; i++) {
        if (a.data[i] != b.data[i]) {
            return i;
        }
    }
    return -1;
}

bool str_equals(struct str a, struct str b)
{
    if (a.len != b.len) {
        return false;
    }

    return str_compare(a, b) == -1; 
}
