#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"
#include "tty.h"

#define STR_(x) #x
#define STR(x) STR_(x)

// TODO: remove
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/**
 * GDT Setup
 * =========
 */
#define GDT_LIMIT_MAX 0xFFFFF

struct gdt_table_entry {
    uint8_t data[8];
};

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

	GDT_ACCESS_DPL_0      = (0<<5),
	GDT_ACCESS_DPL_1      = (1<<5),
	GDT_ACCESS_DPL_2      = (2<<5),
	GDT_ACCESS_DPL_3      = (3<<5),

	GDT_ACCESS_PRESENT    = (1<<7),
};

enum gdt_flags : uint8_t {
	GDT_RESERVED    = (1<<0),
	GDT_LONG        = (1<<1),
	GDT_SIZE        = (1<<2),
	GDT_GRANULARITY = (1<<3),
};

static inline uint16_t gdt_segment_selector(uint16_t index, uint16_t table_indicator, uint16_t priv_level)
{
    return (index            << 3)
         | ((table_indicator << 2) & 0b100)
         | ((priv_level      << 0) & 0b011);
}

struct gdt_entry_content {
    uint32_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
};

static void gdt_encode_entry(struct gdt_table_entry* target, struct gdt_entry_content content)
{
	if (content.limit > GDT_LIMIT_MAX)
		panic(str_attach("GDT cannot encode limits larger than " STR(GDT_LIMIT_MAX)));

	// encode the limit
	target->data[GDT_SDESC_LIMIT_0] = (content.limit >>  0) & 0xFF;
	target->data[GDT_SDESC_LIMIT_1] = (content.limit >>  8) & 0xFF;
	target->data[GDT_SDESC_LIMIT_2] = (content.limit >> 16) & 0xFF;

	// encode the base
	target->data[GDT_SDESC_BASE_0] = (content.base >>  0) & 0xFF;
	target->data[GDT_SDESC_BASE_1] = (content.base >>  8) & 0xFF;
	target->data[GDT_SDESC_BASE_2] = (content.base >> 16) & 0xFF;
	target->data[GDT_SDESC_BASE_3] = (content.base >> 24) & 0xFF;

	// encode the access byte
	target->data[GDT_SDESC_ACCESSBYTE] = content.access_byte;

	// encode the flags
	target->data[GDT_SDESC_FLAGS] |= (content.flags << 4);
}

static void gdt_set(uint16_t limit, uint32_t base, uint32_t offset)
{
    base += offset;

	/* the lgdt instruction requires a packed alignment */
    struct __attribute__((packed)) gdtr  {
        uint16_t limit;
        uint32_t base;
    };

    static struct gdtr gdtr;
    gdtr.limit = limit;
    gdtr.base  = base;

    __asm__ volatile (
        "lgdt %[gdtr]"
        :
        : [gdtr] "m"(gdtr)
    );
}

// only compiles with O1 or higher ¯\_(ツ)_/¯, might have to make this a macro
static inline void gdt_reload(uint32_t code,
                              uint32_t data,
                              uint32_t extra_segment,
                              uint32_t general_segment_1,
                              uint32_t general_segment_2,
                              uint32_t stack_segment)
{
    __asm__ volatile (
        // Far jump to reload the CS register
        "jmp %[cs], $reload_CS\n"
        "reload_CS:\n"
        // Load data segment into AX and then move it to all segment registers
        //"movw %[ds], %%ax\n"
        "movw %[ds],  %%ds\n"
        "movw %[es],  %%es\n"
        "movw %[fs],  %%fs\n"
        "movw %[gs],  %%gs\n"
        "movw %[ss],  %%ss\n"
        : // No output operands
        : [cs] "i"(code),
          [ds] "i"(data),
          [es] "i"(extra_segment),
          [fs] "i"(general_segment_1),
          [gs] "i"(general_segment_2),
          [ss] "i"(stack_segment)
        : "ax" // Clobbered register
    );
}

// sets a flat model
void test_gdt()
{
    // disable interupts
    __asm__ volatile ("cli");

    static struct gdt_table_entry gdt_table[6];
    size_t i=0;
    _Static_assert(sizeof(gdt_table) == 0x30);

    // Null Descriptor
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){0});

    // Kernel mode code segment
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    // Kernel mode data segment
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    // User mode code segment
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    // User mode data segment
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    // Task state segment
    gdt_encode_entry(&gdt_table[i++], (struct gdt_entry_content){
                     .base = &tss,
                     .limit = sizeof(tss)-1,
                     .access_byte = GDT_ACCESS_ACCESSED
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = 0});

    gdt_set(0, 0, 0);
    const uint16_t code_segment = 0x8; // placeholder
    const uint16_t data_segment = 0x10; // placeholder
    gdt_reload(code_segment, data_segment);
}

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


// TODO: remove
#pragma GCC diagnostic pop
