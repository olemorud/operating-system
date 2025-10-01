#pragma once

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "macros.h"

struct bitmap {
    size_t bit_count;
    uint32_t* data;
};

/* buf:      buffer to use for bitmap
 * buf_size: size of buffer in bytes */
#define BITMAP_ATTACH(buf, buf_size) (struct bitmap){.bit_count = buf_size * CHAR_BIT, .data = buf}

/*
 * Set the bit value at `index`
 * */
static inline int bitmap_set(struct bitmap* bitmap, size_t index)
{
    constexpr size_t bits_per_index = sizeof (bitmap->data[0]) * CHAR_BIT;
    if (unlikely(index > bitmap->bit_count)) {
        return -1;
    }
    bitmap->data[index / bits_per_index] |= (1 << (index % bits_per_index));
    return 0;
}

/*
 * Unset the bit value at `index`
 * */
static inline int bitmap_unset(struct bitmap* bitmap, size_t index)
{
    constexpr size_t bits_per_index = sizeof (bitmap->data[0]) * CHAR_BIT;
    if (unlikely(index > bitmap->bit_count)) {
        return -1;
    }
    bitmap->data[index / bits_per_index] &= ~(1 << (index % bits_per_index));
    return 0;
}

/*
 * Get the bit value at `index`
 * */
static inline int bitmap_get(const struct bitmap* bitmap, size_t index)
{
    constexpr size_t bits_per_index = sizeof (bitmap->data[0]) * CHAR_BIT;
    if (unlikely(index > bitmap->bit_count)) {
        return -1; 
    }
    return !!(bitmap->data[index / bits_per_index] & (1ULL << (index % bits_per_index)));
}

/*
 * Finds the index of a bit with value `val`.
 * Returns -1 if none are found
 * */
int bitmap_find(const struct bitmap* bitmap, int val);

/*
 * Sets the range begin..end, end exclusive
 */
int bitmap_set_range(struct bitmap* bitmap, size_t begin, size_t end);

/*
 * Clears the range begin..end, end exclusive
 */
int bitmap_clear_range(struct bitmap* bitmap, size_t begin, size_t end);

/*
 * Returns true if a range is empty
 */
int bitmap_range_empty(struct bitmap* bitmap, size_t begin, size_t end);
