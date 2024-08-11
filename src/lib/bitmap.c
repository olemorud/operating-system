
#include "bitmap.h"
#include <limits.h>

static constexpr struct bitmap B = {0};
static constexpr size_t bits_per_index = sizeof (B.data[0]) * CHAR_BIT;

#define mask(n) ((1<<(n))-1)
_Static_assert(mask(0) == 0b0000);
_Static_assert(mask(1) == 0b0001);
_Static_assert(mask(2) == 0b0011);
_Static_assert(mask(3) == 0b0111);

static inline int trailing_zeroes(int n) 
{
    return __builtin_ctz(n);
}

int bitmap_set_range(struct bitmap* bitmap, size_t begin, size_t end)
{
    if (end < begin) {
        return bitmap_set_range(bitmap, end, begin);
    }

    if (unlikely(begin > bitmap->bit_count)) {
        return -1;
    }

    if (unlikely(end > bitmap->bit_count)) {
        return -2;
    }
 
    /* begin and end are within the same index: */
    if ((begin / bits_per_index) == (end / bits_per_index)) {
        #define expr(begin, end) (~mask((begin) % bits_per_index) & mask((end) % bits_per_index))
        _Static_assert(expr(1,6) == 0b0000111110, "bitmap range logic invalid!");
        bitmap->data[begin / bits_per_index] |= expr(begin, end);
        #undef expr
    } else {
        bitmap->data[begin / bits_per_index] |= ~mask(begin % bits_per_index);
        for (size_t i = 1 + begin/bits_per_index;
             i < end/bits_per_index;
             i++)
        {
            bitmap->data[i] |= ~0;
        }
        bitmap->data[end / bits_per_index] |= mask(end % bits_per_index);
    }

    return 0;
}

int bitmap_clear_range(struct bitmap* bitmap, size_t begin, size_t end)
{
    if (end < begin) {
        return bitmap_set_range(bitmap, end, begin);
    }

    if (unlikely(begin > bitmap->bit_count)) {
        return -1;
    }

    if (unlikely(end > bitmap->bit_count)) {
        return -2;
    }
 
    /* begin and end are within the same index: */
    if ((begin / bits_per_index) == (end / bits_per_index)) {
        #define expr(begin, end) (~mask((begin) % bits_per_index) & mask((end) % bits_per_index))
        _Static_assert(expr(1,6) == 0b0000111110, "bitmap range logic invalid!");
        bitmap->data[begin / bits_per_index] &= ~expr(begin, end);
        #undef expr
    } else {
        bitmap->data[begin / bits_per_index] &= mask(begin % bits_per_index);
        for (size_t i = 1 + begin/bits_per_index;
             i < end/bits_per_index;
             i++)
        {
            bitmap->data[i] = 0;
        }
        bitmap->data[end / bits_per_index] &= ~mask(end % bits_per_index);
    }

    return 0;
}

/* This 4 am code is almost certainly wrong. No tests have been written yet.
 *
 * Returns 0 if range is empty. 
 * If range is not emtpy, it returns the offset of the first non-zero bit
 * from `begin`
 *
 * TODO: write tests */
int bitmap_range_empty(struct bitmap* bitmap, size_t begin, size_t end)
{
    if (end < begin) {
        return bitmap_set_range(bitmap, end, begin);
    }

    if (unlikely(begin > bitmap->bit_count)) {
        return -1;
    }

    if (unlikely(end > bitmap->bit_count)) {
        return -2;
    }
 
    /* begin and end are within the same index: */
    if ((begin / bits_per_index) == (end / bits_per_index)) {
        #define expr(begin, end) (~mask((begin) % bits_per_index) & mask((end) % bits_per_index))
        _Static_assert(expr(1,6) == 0b0000111110, "bitmap range logic invalid!");
        const typeof(bitmap->data[0]) n = bitmap->data[begin / bits_per_index];
        if (n & expr(begin, end)) {
            return trailing_zeroes(n) - (begin % bits_per_index) ;
        };
        #undef expr
    } else {
        {
            const typeof(bitmap->data[0]) n = bitmap->data[begin / bits_per_index];
            if (n & ~mask(begin % bits_per_index)) {
                return trailing_zeroes(n) - (begin % bits_per_index);
            }
        }

        for (size_t i = 1 + begin/bits_per_index;
             i < end/bits_per_index;
             i++)
        {
            const typeof(bitmap->data[0]) n = bitmap->data[i];
            if (n != 0) {
                return i*bits_per_index + trailing_zeroes(n);
            }
        }

        {
            const typeof(bitmap->data[0]) n = bitmap->data[end / bits_per_index];
            if (n & mask(end % bits_per_index)) {
                return (end % bits_per_index) - trailing_zeroes(n);
            }
            bitmap->data[end / bits_per_index] &= ~mask(end % bits_per_index);
        }
    }

    return 0;
}
