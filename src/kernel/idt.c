
#include <stdint.h>

#include "idt.h"

struct idt_gate_descriptor idt_encode_descriptor(void* func, segment_t segment_selector, enum idt_dpl dpl, enum idt_flags type)
{
    struct idt_gate_descriptor output = {0};

	uint32_t offset = (uint32_t)func;

    output.offset_0 = offset & 0xFFFF;
    offset >>= 16;
    output.offset_1 = offset & 0xFFFF;

    output.segment_selector = segment_selector;
    output.flags = type | dpl | IDT_PRESENT;

    return output;
}

/*
 *  size: one less than the idt in bytes
 *  offset: linear address of the idt
 * */
void idt_load(struct idt_gate_descriptor table[], uint16_t size)
{
    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t offset;
    } idt = {
        .size = size,
        .offset = (uint32_t)table,
    };
    _Static_assert(sizeof idt.size == 2);
    _Static_assert(sizeof idt.offset == 4);
    _Static_assert(sizeof idt == 6);

    __asm__ volatile (
        "lidt %[idt]"
        :
        : [idt] "m"(idt)
    );
}
