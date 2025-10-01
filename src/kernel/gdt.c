#include "gdt.h"
#include "libc.h"
#include "str.h"

inline uint16_t gdt_segment_selector(uint16_t index, uint16_t table_indicator, uint16_t priv_level)
{
    return (index            << 3)
         | ((table_indicator << 2) & 0b100)
         | ((priv_level      << 0) & 0b011);
}

struct gdt_table_entry gdt_encode_entry(struct gdt_entry_content content)
{
    if (content.limit > GDT_LIMIT_MAX)
        panic(str_attach("GDT cannot encode limits larger than " CSTR(GDT_LIMIT_MAX)));

    struct gdt_table_entry entry;

    entry.data[GDT_SDESC_LIMIT_0] = (content.limit >>  0) & 0xFF;
    entry.data[GDT_SDESC_LIMIT_1] = (content.limit >>  8) & 0xFF;
    entry.data[GDT_SDESC_LIMIT_2] = (content.limit >> 16) & 0xFF;

    entry.data[GDT_SDESC_BASE_0] = (content.base >>  0) & 0xFF;
    entry.data[GDT_SDESC_BASE_1] = (content.base >>  8) & 0xFF;
    entry.data[GDT_SDESC_BASE_2] = (content.base >> 16) & 0xFF;
    entry.data[GDT_SDESC_BASE_3] = (content.base >> 24) & 0xFF;

    entry.data[GDT_SDESC_ACCESSBYTE] = content.access_byte;

    entry.data[GDT_SDESC_FLAGS] |= (content.flags << 4);

    return entry;
}

//void gdt_load(uint16_t size, const struct gdt_table_entry base[], uint32_t offset)
//{
//    base += offset;
//
//    /* the lgdt instruction requires a packed alignment */
//    struct __attribute__((packed)) gdtr  {
//        uint16_t size;
//        uint32_t base;
//    } gdt = {
//        .size = size,
//        .base = (uint32_t)base,
//    };
//
//    __asm__ volatile (
//        "lgdt %[gdt]"
//        :
//        : [gdt] "m"(gdt)
//    );
//}

void gdt_flush_granular(uint32_t code,
                        uint32_t data,
                        uint32_t extra_segment,
                        uint32_t general_segment_1,
                        uint32_t general_segment_2,
                        uint32_t stack_segment)
{
    __asm__ volatile (
        // Far jump to reload the CS register
		"push %[cs]\n"
		"push $reload_CS_granular\n"
		"retf\n"

        "reload_CS_granular:\n"
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
        : [cs] "m"(code),
          [ds] "m"(data),
          [es] "m"(extra_segment),
          [fs] "m"(general_segment_1),
          [gs] "m"(general_segment_2),
          [ss] "m"(stack_segment)
        : "ax" // Clobbered register
    );
}

void gdt_load(struct gdt_table_entry base[], uint16_t gdt_size, segment_t data, segment_t code)
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
		"push %[cs]\n"
		"push $reload_CS\n"
		"retf\n"

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
		  [ds]  "m"(data),
          [cs]  "m"(code)
        : "ax" // Clobbered register
    );
}
