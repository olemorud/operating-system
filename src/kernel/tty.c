/*
 * Inspired by OSDev - Bare Bones
 * 	URL: 	 https://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
 *	Archive: https://web.archive.org/web/20240718110628/https://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
 *	Date:	 2024-07-29
 */

#include "tty.h"
#include "libc.h"

static struct terminal_state t = {
    .row    = 0,
    .column = 0,
    .color  = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK),
    .buf    = (uint16_t*)0xB8000,
};

static inline bool isprint(int c)
{
    return (c >= ' ') && (c <= '~');
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_clear()
{
    t.row = 0,
    t.column = 0,
    t.color = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        t.buf[i] = vga_entry(' ', t.color);
    }
}

void terminal_set_color(uint8_t fg, uint8_t bg)
{
    t.color = vga_color(fg, bg);
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	terminal_buf[y*VGA_WIDTH + x] = vga_entry(c, color);
}

void terminal_scroll(int n)
{
	constexpr size_t buf_size = VGA_WIDTH * VGA_HEIGHT; 

	const size_t offset = VGA_WIDTH * n;
	const size_t len = buf_size - offset;
    memmove(terminal_buf,
			terminal_buf + offset,
			len * sizeof *terminal_buf);
    for (size_t i = len; i < buf_size; i++) {
        terminal_buf[i] = vga_entry(' ', t.color);
    }
    t.row -= n;
}

void terminal_putchar(int c)
{
	/* clear the cursor marker */
	terminal_putentryat(' ', t.color, t.column, t.row);

    // TODO: implement other control characters
    switch (c) {

    case '\n':
        t.column = 0;
        t.row += 1;
        break;

    default:
        c = isprint(c) ? c : '?';
        terminal_putentryat(c, t.color, t.column, t.row);
        t.column += 1;
        if (t.column == VGA_WIDTH) {
            t.column = 0;
            t.row += 1;
        }
    }

    if (t.row == VGA_HEIGHT) {
        terminal_scroll(1);
    }

	/* set the cursor marker */
	terminal_putentryat(' ', vga_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY), t.column, t.row);
}

void terminal_write(struct str str)
{
	for (size_t i = 0; i < str.len; i++) {
		terminal_putchar(str.data[i]);
    }
}

