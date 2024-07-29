/*
 * Inspired by OSDev - Bare Bones
 * 	URL: 	 https://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
 *	Archive: https://web.archive.org/web/20240718110628/https://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
 *	Date:	 2024-07-29
 */

#pragma once

#include <stdint.h>
#include "str.h"

static constexpr size_t VGA_WIDTH = 80;
static constexpr size_t VGA_HEIGHT = 25;

static uint16_t* const terminal_buf = (uint16_t*)0xB8000;

struct [[nodiscard]] terminal_state {
    size_t          row;
    size_t          column;
    uint8_t         color;
    uint16_t* const buf;
};

/* Hardware text mode color constants. */
enum vga_color : uint8_t {
	VGA_COLOR_BLACK         = 0,
	VGA_COLOR_BLUE          = 1,
	VGA_COLOR_GREEN         = 2,
	VGA_COLOR_CYAN          = 3,
	VGA_COLOR_RED           = 4,
	VGA_COLOR_MAGENTA       = 5,
	VGA_COLOR_BROWN         = 6,
	VGA_COLOR_LIGHT_GREY    = 7,
	VGA_COLOR_DARK_GREY     = 8,
	VGA_COLOR_LIGHT_BLUE    = 9,
	VGA_COLOR_LIGHT_GREEN   = 10,
	VGA_COLOR_LIGHT_CYAN    = 11,
	VGA_COLOR_LIGHT_RED     = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN   = 14,
	VGA_COLOR_WHITE         = 15,
};

#define vga_color(fg, bg) (fg | bg << 4)

void terminal_clear();

void terminal_set_color(uint8_t fg, uint8_t bg);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

void terminal_scroll(int n);

void terminal_putchar(int c);

void terminal_write(struct str str);
