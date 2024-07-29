#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"
#include "tty.h"

void kernel_main(void)
{
	terminal_clear();

    for (size_t i = 0; i < 15; i++) {
        printf(str_attach("hello {u32}!\n"), i);
    }
}

