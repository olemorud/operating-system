
#include "ring_buffer.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

struct myObj {
    int32_t a;
    int32_t b;
};

static int g_status = EXIT_SUCCESS;
static int test_no = 0;
static bool test_ongoing = false;

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
    struct ring_buffer q = make_ring_buffer(struct myObj);

    test_begin("entry_size");
    {
        if (q.entry_size != sizeof (struct myObj)) {
            test_fail("wrong entry_size");
        } else {
            test_ok("entry_size is %zu", q.entry_size);
        }
    }

    test_begin("ring_buffer_remaining");
    size_t max_items = ring_buffer_remaining(&q); // used later
    {
        if (max_items != RING_BUFFER_MAX / sizeof(struct myObj)) {
            test_fail("unexpected ring_buffer_remaining()");
        } else {
            test_ok("ring_buffer_remaining() is %zu", max_items);
        }
    }

    test_begin("ring_buffer_push");
    {
        bool ok = true;
        size_t remaining = ring_buffer_remaining(&q);
        for (size_t i = 0; i < remaining; i++) {
            if (ring_buffer_push(&q, &(struct myObj){i, i}) == false) {
                ok = false;
                break;
            }
        } 
        if (!ok) {
            test_fail("expected ring_buffer push to work");
        } else {
            test_ok("successfully pushed %zu times", remaining);
        }
    }

    test_begin("ring_buffer_push on full ring buffer");
    {
        if (ring_buffer_push(&q, &(struct myObj){}) == true) {
            test_fail("ring buffer should be full by now, ring_buffer_remaining is %zu", ring_buffer_remaining(&q));
        } else {
            test_ok("correctly failed to push when ring buffer is full");
        }
    }

    test_begin("ring_buffer_get");
    {
        for (size_t i = 0; i < max_items; i++) {
            struct myObj o;
            bool ok = ring_buffer_get(&q, &o);

            if (!ok) {
                test_fail("ring_buffer_get() didn't fetch when buffer had data");
                goto end_test;
            }

            if (o.a != (int32_t)i
             || o.b != (int32_t)i
            ){
                test_fail("ring_buffer_get() retrived faulty data {%i, %i}", o.a, o.b); 
                goto end_test;
            }
        }
        test_ok("ring_buffer_get() works");

end_test:
    }

    test_begin("ring_buffer_get on empty ring buffer");
    {
        struct myObj o;
        bool ok = ring_buffer_get(&q, &o);
        if (ok) {
            test_fail("ring_buffer_get() on empty ring buffer succeeded, got {%i, %i}", o.a, o.b);
        } else {
            test_ok("ring_buffer_get() on empty ring buffer didn't succeed");
        }
    }

    test_begin("stress test");
    {
        struct ring_buffer rb = make_ring_buffer(struct myObj);
        bool fail = false;

        for (volatile size_t i = 0; i < RING_BUFFER_MAX * 10; i++) {
            bool ok;
            struct myObj before = {i, i};
            struct myObj after = {-1, -1};

            ok = ring_buffer_push(&rb, &before);
            if (!ok) {
                test_fail("failed to push during stress-test");
                fail = true;
                break;
            }

            ok = ring_buffer_get(&rb, &after);
            if (!ok) {
                test_fail("failed to get during stress-test");
                fail = true;
                break;
            }

            if (before.a != after.a || before.b != after.b) {
                test_fail("data got damaged in queue");
                fail = true;
                break;
            }
        }
        if (!fail) {
            test_ok("stress test succeeded");
        }
    }

    if (g_status == EXIT_SUCCESS) {
        printf("\nAll tests succeeded\n");
    } else {
        printf("\nOne or more tests failed\n");
    }

    return g_status;
}
