
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

static int print_i32(struct printf_state* s)
{
    long n = va_arg(s->ap, int32_t);
    return print_long(n, str_attach("0123456789"), true);
}

static int print_u32(struct printf_state* s)
{
    unsigned long n = va_arg(s->ap, uint32_t);
    return print_long(n, str_attach("0123456789"), false);
}

static int print_x32(struct printf_state* s)
{
    unsigned long n = va_arg(s->ap, uint32_t);
    return print_long(n, str_attach("0123456789abcdefgh"), false);
}

static int print_str(struct printf_state* s)
{
	const struct str str = va_arg(s->ap, struct str);
	for (size_t i=0; i < str.len; i++) {
		terminal_putchar(str.data[i]);
	}
	return str.len;
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
		// 16 bit types are promoted to 32 bit by va_arg(...) anyways
        case 'i16':
        case 'i32':
			return print_i32(s);

        case 'u16':
        case 'u32':
			return print_u32(s);

        case 'x16':
        case 'x32':
			return print_x32(s);

		case 'str':
			return print_str(s);

        default:
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
