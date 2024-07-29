
#include <stdarg.h>
#include <limits.h>

#include "libc.h"

typedef int (*printf_function)(struct printf_state* s, void* data);

constexpr int EOF = -1;

struct printf_state {
    struct str str;
    va_list ap;
    size_t i;
    int written;
};

static inline int ps_peek(struct printf_state* s)
{
    if (s->i == s->str.len) {
        return EOF;
    }
    return s->str.data[s->i];
}

static inline int ps_get(struct printf_state* s)
{
    if (s->i == s->str.len) {
        return EOF;
    }
    return s->str.data[s->i++];
}

static int print_long(unsigned long n, struct str alphabet, bool is_signed)
{
    constexpr size_t BUF_SZ = 21;
    char buf[BUF_SZ];
    size_t i = 0;

    if (n == 0) {
        terminal_putchar('0');
        return 1;
    }

    bool is_negative = false;
    if (is_signed) {
        signed long signed_n = (signed long)n;
        if (signed_n < 0) {
            is_negative = true;
            n = -signed_n;
        }
    }

    if (is_negative)
        terminal_putchar('-');

    while (n) {
        i += 1;
        buf[BUF_SZ-i] = alphabet.data[n % alphabet.len];
        n /= alphabet.len;
    }

    terminal_write((struct str){.data = &buf[BUF_SZ-i], .len = i});

    return i;
}   

static int print_i16(struct printf_state* s)
{
    // int16_t is promoted to 'int' when passed through '...'
    long n = va_arg(s->ap, int32_t);
    return print_long(n, str_attach("0123456789"), true);
}

static int print_i32(struct printf_state* s)
{
    long n = va_arg(s->ap, int32_t);
    return print_long(n, str_attach("0123456789"), true);
}

static int print_i64(struct printf_state* s)
{
    panic(str_attach("print_i64 not implemented on i686"));
    long n = va_arg(s->ap, int64_t);
    return print_long(n, str_attach("0123456789"), true);
}

static int print_u16(struct printf_state* s)
{
    // uint16_t is promoted to 'unsigned int' when passed through '...'
    unsigned long n = va_arg(s->ap, unsigned int);
    return print_long(n, str_attach("0123456789"), false);
}

static int print_u32(struct printf_state* s)
{
    unsigned long n = va_arg(s->ap, uint32_t);
    return print_long(n, str_attach("0123456789"), false);
}

static int print_u64(struct printf_state* s)
{
    panic(str_attach("print_u64 not implemented on i686"));
    unsigned long n = va_arg(s->ap, uint64_t);
    return print_long(n, str_attach("0123456789"), false);
}

#pragma GCC diagnostic ignored "-Wmultichar"
static int parse_format_cmd(struct printf_state* s)
{
    int c;
    uint32_t cmd = 0;

    constexpr uint32_t CHAR_MASK = (1<<CHAR_BIT)-1;

    while (c = ps_get(s), c != EOF && c != '}') {
        cmd <<= CHAR_BIT;
        cmd |= c & CHAR_MASK;
    }

    if (c == EOF)
        return -1;

    switch (cmd) {
        case 'i16': return print_i16(s);
        case 'i32': return print_i32(s);
        case 'i64': return print_i64(s);
        case 'u16': return print_u16(s);
        case 'u32': return print_u32(s);
        case 'u64': return print_u64(s);
        default:
            terminal_write(str_attach("\nunknown cmd: "));
            while (cmd) {
                terminal_putchar(cmd & CHAR_MASK);
                cmd >>= CHAR_BIT;
            }
            terminal_putchar('\n');
            return -1;
    }
}

int printf(struct str format, ...)
{
    int c;

    struct printf_state s = {
        .str = format,
        .i = 0,
        .written = 0,
    };
    va_start(s.ap, format);

    while ((c = ps_get(&s)) != EOF) {
        switch (c) {

        case '{':
            int ok = parse_format_cmd(&s);
            if (ok == -1)
                return -1;
            s.written += ok;
            break;

        default:
            terminal_putchar(c);
            s.written += 1;
            break;
        }
    }

    return s.written;
}
