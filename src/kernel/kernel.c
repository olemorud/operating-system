#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"
#include "tty.h"

#define STR_(x) #x
#define STR(x) STR_(x)

/**
 * GDT Setup
 * =========
 */
enum gdt_segment_descriptor_index : size_t {
	GDT_SDESC_BASE_0 = 2,
	GDT_SDESC_BASE_1 = 3,
	GDT_SDESC_BASE_2 = 4,
	GDT_SDESC_BASE_3 = 7,

	GDT_SDESC_LIMIT_0 = 0,
	GDT_SDESC_LIMIT_1 = 1,
	GDT_SDESC_LIMIT_2 = 6,

	GDT_SDESC_ACCESSBYTE = 5,

	GDT_SDESC_FLAGS = 6,
};

enum gdt_access_flag : uint8_t {
	GDT_ACCESS_ACCESSED   = (1<<0),
	GDT_ACCESS_RW 		  = (1<<1),
	GDT_ACCESS_DC 		  = (1<<2),
	GDT_ACCESS_EXEC       = (1<<3),
	GDT_ACCESS_DESCRIPTOR = (1<<4),
	GDT_ACCESS_DPL        = (1<<5),
	GDT_ACCESS_PRESENT    = (1<<6),
};

enum gdt_flags : uint8_t {
	GDT_RESERVED    = (1<<0),
	GDT_LONG        = (1<<1),
	GDT_SIZE        = (1<<2),
	GDT_GRANULARITY = (1<<3),
}

struct gdt {
	uint32_t base;
	uint32_t limit;
	uint8_t  access_byte;
   	uint8_t  flags;
};

static void gdt_encode_entry(uint8_t *target, struct GDT source)
{
#define GDT_LIMIT_MAX 0xFFFFF
	if (source.limit > GDT_LIMIT_MAX)
		panic(str_attach("GDT cannot encode limits larger than " STR(GDT_LIMIT_MAX)));

	// encode the limit
	target[GDT_SDESC_LIMIT_0] = (source.limit >>  0) & 0xFF;
	target[GDT_SDESC_LIMIT_1] = (source.limit >>  8) & 0xFF;
	target[GDT_SDESC_LIMIT_2] = (source.limit >> 16) & 0xFF;

	// encode the base
	target[GDT_SDESC_BASE_0] = (source.base >>  0) & 0xFF;
	target[GDT_SDESC_BASE_1] = (source.base >>  8) & 0xFF;
	target[GDT_SDESC_BASE_2] = (source.base >> 16) & 0xFF;
	target[GDT_SDESC_BASE_3] = (source.base >> 24) & 0xFF;

	// encode the access byte
	target[GDT_SDESC_ACCESSBYTE] = source.access_byte;

	// encode the flags
	target[GDT_SDESC_FLAGS] |= (source.flags << 4);
}

void gdt_set(uint16_t limit, uint32_t base)
{
	/* the lgdt instruction requires a packed alignment */
    struct __attribute__((packed)) gdtr  {
        uint16_t limit;
        uint32_t base;
    };

    struct gdtr gdtr = {
        .limit = limit,
        .base = base,
    };

    __asm__ volatile (
        "lgdt %0"
        :
        : "m" (gdtr));
}

void gdt_segment_reload()

/**
 * Kernel entrypoint
 * =================
 */
void kernel_main(void)
{
	terminal_clear();

    for (size_t i = 0; i < VGA_HEIGHT / 2 + 1; i++) {
        printf(str_attach("hello {u32}!\n"), i);
    }
}

