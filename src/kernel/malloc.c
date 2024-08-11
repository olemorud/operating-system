
#include <limits.h>

#include "kernel_state.h"
#include "libc.h"

static constexpr size_t HEAP_SIZE = 1024*1024;
static constexpr size_t HEAP_ALIGN = 16;

static uint8_t heap[HEAP_SIZE] = {0};

#if 0
static uint8_t bitmap[HEAP_SIZE / CHAR_BIT] = {0};

static inline void bitmap_set(size_t i)
{
    bitmap[i / CHAR_BIT] |= 1<<(i%CHAR_BIT);
}

static inline void bitmap_clear(size_t i)
{
    bitmap[i / CHAR_BIT] &= ~(1<<(i%CHAR_BIT));
}

static inline bool bitmap_get(size_t i)
{
    return bitmap[i / CHAR_BIT] & (1<<(i%CHAR_BIT));
}
#endif

void* kalloc(size_t size)
{
    static int n = 0;

    if (n + size > HEAP_SIZE) {
        panic(str_attach("no more heap space!\n"));
    }

    void* ptr = &(heap[n]);
    n += size;
    return ptr;
}

void kfree(void* ptr)
{
    /* do nothing, lol */
    (void)ptr;
}

void* krealloc(void* ptr, size_t size)
{
    void* new = kalloc(size);
    if (new == NULL)
        return NULL;
    memmove(new, ptr, size); // this overflows but it's ok
    kfree(ptr);
    return new;
}
