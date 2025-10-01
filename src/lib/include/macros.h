
#pragma once

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define MEMBER_TYPE(parent_type, member) \
    typeof(((parent_type){0}).member)

#define MEMBER_SIZE(type, member) \
    (sizeof (((type){0}).member))
