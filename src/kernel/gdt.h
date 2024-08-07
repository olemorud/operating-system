#pragma once
#include <stdint.h>
#include "types.h"

#define GDT_LIMIT_MAX 0xFFFFF

struct gdt_table_entry { 
    volatile uint8_t data[8];
};
_Static_assert(sizeof(struct gdt_table_entry) == 8);

struct gdt_entry_content {
    volatile uint32_t base;
    volatile uint32_t limit;
    volatile uint8_t access_byte;
    volatile uint8_t flags;
};

enum gdt_segment_descriptor_index {
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
    GDT_ACCESS_RW         = (1<<1),
    GDT_ACCESS_DC         = (1<<2),
    GDT_ACCESS_EXEC       = (1<<3),
    GDT_ACCESS_DESCRIPTOR = (1<<4),

    GDT_ACCESS_DPL_0      = (0<<5),
    GDT_ACCESS_DPL_1      = (1<<5),
    GDT_ACCESS_DPL_2      = (2<<5),
    GDT_ACCESS_DPL_3      = (3<<5),

    GDT_ACCESS_PRESENT    = (1<<7),
};

enum gdt_flags : uint8_t {
    GDT_RESERVED             = (1<<0),
    GDT_LONG                 = (1<<1),
    GDT_32BIT                = (1<<2),
    GDT_GRANULARITY_PAGEWISE = (1<<3),
};

struct gdt_table_entry gdt_encode_entry(struct gdt_entry_content content);

// only compiles with O1 or higher ¯\_(ツ)_/¯, might have to make this a macro
static inline void gdt_flush_granular(uint32_t code,
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
        "movw %[ds], %%ax\n"
        "movw %%ax,  %%ds\n"
        "movw %[es], %%ax\n"
        "movw %%ax,  %%es\n"
        "movw %[fs], %%ax\n"
        "movw %%ax,  %%fs\n"
        "movw %[gs], %%ax\n"
        "movw %%ax,  %%gs\n"
        "movw %[ss], %%ax\n"
        "movw %%ax,  %%ss\n"
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
static inline void gdt_load(struct gdt_table_entry base[], uint16_t gdt_size, segment_t data, segment_t code)
{
    /* the lgdt instruction requires a packed alignment */
    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t base;
    } gdt = {
        .size = gdt_size,
        .base = (uint32_t)base,
    };

    __asm__ volatile (
		"lgdt %[gdt]\n"
        // Far jump to reload the CS register
        "jmp %[cs], $reload_CS\n"
        "reload_CS:\n"
        // Load data segment into AX and then move it to all segment registers
        "movw %[ds], %%ax\n"
        "movw %%ax,  %%ds\n"
        "movw %%ax,  %%es\n"
        "movw %%ax,  %%fs\n"
        "movw %%ax,  %%gs\n"
        "movw %%ax,  %%ss\n"
        : // No output operands
        : [gdt] "m"(gdt),
		  [ds] "i"(data),
          [cs] "i"(code)
        : "ax" // Clobbered register
    );
}
