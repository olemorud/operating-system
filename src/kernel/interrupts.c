#include <stdint.h>
#include "libc.h"

#ifdef __x86_64__
typedef unsigned long long int uword_t;
#else
typedef unsigned int uword_t;
#endif

struct __attribute__((packed)) interrupt_frame {
    uword_t ip;
    uword_t cs;
    uword_t flags;
    uword_t sp;
    uword_t ss;
};

__attribute__((interrupt)) void interrupt_handler_1(struct interrupt_frame* frame)
{
	(void)frame;
}
