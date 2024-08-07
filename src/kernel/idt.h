#pragma once

#include <stdint.h>
#include "types.h"

struct __attribute__((packed)) idt_gate_descriptor {
    uint16_t offset_0;
    uint16_t segment_selector;

    uint8_t ist; // unused in 32-bit

    /* `flags`
       =======
       bit 0-3: gate type
       bit   4: unused
       bit 5-6: dpl
       bit   7: present bit
      
       DPL is ignored by hardware interupts and present bit must be 1 */
    uint8_t flags;

    uint16_t offset_1;
};

_Static_assert(sizeof(struct idt_gate_descriptor) == 8);

enum idt_flags : uint8_t {
    IDT_GATE_TYPE_TASK        = 0x5,
    IDT_GATE_TYPE_INTERRUPT16 = 0x6,
    IDT_GATE_TYPE_TRAP16      = 0x7,
    IDT_GATE_TYPE_INTERRUPT32 = 0xE,
    IDT_GATE_TYPE_TRAP32      = 0xF,
    
    IDT_PRESENT = 1 << 7,
};

enum idt_dpl : uint8_t {
    IDT_DPL_0 = 0 << 5,
    IDT_DPL_1 = 1 << 5,
    IDT_DPL_2 = 2 << 5,
    IDT_DPL_3 = 3 << 5,
};

struct idt_gate_descriptor idt_encode_descriptor(void* func, segment_t segment_selector, enum idt_dpl dpl, enum idt_flags type);

void idt_load(struct idt_gate_descriptor table[], uint16_t size);

