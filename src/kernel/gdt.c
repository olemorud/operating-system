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

