#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "bitmap.h"

static int g_status = EXIT_SUCCESS;
static int test_no = 0;
static bool test_ongoing = false;

static void bitmap_print(struct bitmap* bitmap)
{
    static constexpr size_t bits_per_index = sizeof (bitmap->data[0]) * CHAR_BIT;
    fprintf(stderr, "printing bitmap... (bit_count: %zu, indices: %zu)\n", bitmap->bit_count, bitmap->bit_count / bits_per_index);
    for (size_t i = 0; i < bitmap->bit_count / bits_per_index; i++) {
        typeof(bitmap->data[0]) n = bitmap->data[i];
        for (size_t i = 0; i < bits_per_index; i++) {
            fprintf(stderr, "%d", n & 1);
            n >>= 1;
        }
    }
    fprintf(stderr, "\n");
}

#define test_begin(name) _test_begin(name, __LINE__)
static void _test_begin(const char* name, int line)
{
    (void)line; // unused for now
    assert(!test_ongoing);
    test_ongoing = true;
    printf("\n#%i - %s:\n", test_no, name);
    test_no += 1;
}

static void test_ok(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("OK: ");
    vprintf(format, ap);
    printf("\n");
    va_end(ap);
    test_ongoing = false;
}

static void test_fail(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("FAIL: ");
    vprintf(format, ap);
    printf("\n");
    va_end(ap);
    g_status = EXIT_FAILURE;
    test_ongoing = false;
}


int main()
{

    test_begin("test bitmap_set()");
    do {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);
        constexpr size_t index = 3;
        int ok = bitmap_set(&b, index);
        if (ok != 0) {
            test_fail("bitmap_set(%zu) returned non-zero value", index);
            break;
        }
        bitmap_print(&b);
        if (b.data[0] != 0b1000) {
            test_fail("bitmap_set(%zu) failed", index);
            break;
        }

        test_ok("bitmap_set(%zu) ok", index);
    } while (0);

    test_begin("test bitmap_get()");
    do {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);
        constexpr size_t index = 3;
        int ok = bitmap_set(&b, index);
        if (ok != 0) {
            test_fail("bitmap_set(%zu) failed when testing bitmap_get(%zu)", index, index);
            break;
        }
        bitmap_print(&b);
        int v = bitmap_get(&b, 3);
        if (v < 0) {
            test_fail("bitmap_get(%zu) returned negative value", index);
            break;
        }
        if (v != 1) {
            test_fail("expected 1, got 0");
            break;
        }
        test_ok("bitmap_get(%zu) ok", index);
    } while (0);

    test_begin("test bitmap_set_range() within one cell");
    {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);

        constexpr size_t begin = 3;
        constexpr size_t end = 6;
        int ok = bitmap_set_range(&b, begin, end);
        if (ok < 0) {
            test_fail("bitmap_set_range returned negative value");
            goto end_set_range;
        }
        bitmap_print(&b);
        for (size_t i = 0; i < begin; i++) {
            if (bitmap_get(&b, i) == 1) {
                test_fail("bitmap_set_range setting bad values");
                goto end_set_range;
            }
        }
        for (size_t i = begin; i < end; i++) {
            if (bitmap_get(&b, i) == 0) {
                test_fail("bitmap_set_range setting bad values");
                goto end_set_range;
            }
        }
        for (size_t i = end; i < b.bit_count; i++) {
            if (bitmap_get(&b, i) == 1) {
                test_fail("bitmap_set_range setting bad values");
                goto end_set_range;
            }
        }
        test_ok("bitmap_set_range(..., 3, 6) ok!");
        end_set_range:
    };

    test_begin("test bitmap_set_range() across two cells");
    {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);

        constexpr size_t begin = 14;
        constexpr size_t end = 14 + 2 * sizeof (b.data[0]) * CHAR_BIT;
        int ok = bitmap_set_range(&b, begin, end);

        if (ok < 0) {
            test_fail("bitmap_set_range returned negative value");
            goto multi_cell_set_range_fail;
        }
        bitmap_print(&b);

        for (size_t i = 0; i < begin; i++) {
            if (bitmap_get(&b, i) != 0) {
                test_fail("bitmap_set_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_set_range_fail;
            }
        }
        for (size_t i = begin; i < end; i++) {
            if (bitmap_get(&b, i) != 1) {
                test_fail("bitmap_set_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_set_range_fail;
            }
        }
        for (size_t i = end; i < b.bit_count; i++) {
            if (bitmap_get(&b, i) != 0) {
                test_fail("bitmap_set_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_set_range_fail;
            }
        }

        test_ok("bitmap_set_range(..., %zu, %zu) ok!", begin, end);

        multi_cell_set_range_fail:
    };

    test_begin("test bitmap_clear_range() within one cell");
    {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);

        memset(data, 0xff, sizeof data);

        constexpr size_t begin = 3;
        constexpr size_t end = 7;
        int ok = bitmap_clear_range(&b, begin, end);
        if (ok < 0) {
            test_fail("bitmap_clear_range(..., %zu, %zu) returned negative value", begin, end);
            goto single_cell_clear_fail;
        }
        bitmap_print(&b);

        for (size_t i = 0; i < begin; i++) {
            if (bitmap_get(&b, i) != 1) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto single_cell_clear_fail;
            }
        }
        for (size_t i = begin; i < end; i++) {
            if (bitmap_get(&b, i) != 0) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto single_cell_clear_fail;
            }
        }
        for (size_t i = end; i < b.bit_count; i++) {
            if (bitmap_get(&b, i) != 1) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto single_cell_clear_fail;
            }
        }

        test_ok("bitmap_clear_range(..., %zu, %zu) ok!", begin, end);
        single_cell_clear_fail:
    }

    test_begin("test bitmap_clear_range() within multiple cells");
    {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);

        memset(data, 0xff, sizeof data);

        constexpr size_t begin = 3;
        constexpr size_t end = begin + 2 * sizeof(b.data[0]) * CHAR_BIT;
        int ok = bitmap_clear_range(&b, begin, end);
        if (ok < 0) {
            test_fail("bitmap_clear_range(..., %zu, %zu) returned negative value", begin, end);
            goto multi_cell_clear_fail;
        }
        bitmap_print(&b);

        for (size_t i = 0; i < begin; i++) {
            if (bitmap_get(&b, i) != 1) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_clear_fail;
            }
        }
        for (size_t i = begin; i < end; i++) {
            if (bitmap_get(&b, i) != 0) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_clear_fail;
            }
        }
        for (size_t i = end; i < b.bit_count; i++) {
            if (bitmap_get(&b, i) != 1) {
                test_fail("bitmap_clear_range(..., %zu, %zu) setting bad values", begin, end);
                goto multi_cell_clear_fail;
            }
        }

        test_ok("bitmap_clear_range(..., %zu, %zu) ok!", begin, end);
        multi_cell_clear_fail:
    }

    test_begin("test bitmap_empty()");
    do {
        static uint32_t data[4] = {0};
        struct bitmap b = BITMAP_ATTACH(data, sizeof data);

        constexpr size_t roadblock = 14;
        int ok = bitmap_set(&b, roadblock);
        if (ok != 0) {
            test_fail("bitmap_set(..., %zu) failed when testing bitmap_empty", roadblock);
            break;
        }
        bitmap_print(&b);

        {
            constexpr size_t begin = 0;
            constexpr size_t end = roadblock - 1;
            size_t v = bitmap_range_empty(&b, begin, end);
            if (v != 0) {
                test_fail("bitmap_range_empty(..., %zu, %zu) returned %d when it should return %d", begin, end, v, 0);
                break;
            }
        }

        {
            constexpr size_t begin = 4;
            constexpr size_t end   = 32;
            int v = bitmap_range_empty(&b, begin, end);
            if (v != roadblock - begin) {
                test_fail("expected bitmap_range_empty(..., %zu, %zu) to return %d, got %d", begin, end, roadblock - begin, v);
                break;
            }
        }

        test_ok("bitmap_range_empty() ok!");

    } while (0);

    return g_status;
}
