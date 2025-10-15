#pragma once

enum {
    SYSCALL_PRINT,
    SYSCALL_OPEN,
    SYSCALL_READ,
    SYSCALL_WRITE,
    SYSCALL_EXIT,
};

#define syscall(number, b, c, d) \
    __asm__ volatile(            \
        "int $0x80\n"            \
        :                        \
        : "a"(number),           \
          "b"(b),                \
          "c"(c),                \
          "d"(d)                 \
        : "memory"               \
    );
