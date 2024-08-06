#pragma once

#include <stdint.h>

struct __attribute__((packed)) idt_gate_descriptor {
    uint16_t offset_0;
    uint16_t segment_selector;
    uint8_t ist;

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

enum idt_flags : uint8_t {
    IDT_GATE_TYPE_TASK_GATE        = 0x5,
    IDT_GATE_TYPE_INTERUPT_GATE_16 = 0x6,
    IDT_GATE_TYPE_TRAP_GATE_16     = 0x7,
    IDT_GATE_TYPE_INTERUPT_GATE_32 = 0xE,
    IDT_GATE_TYPE_TRAP_GATE_32     = 0xF,
    
    IDT_DPL_0 = 0 << 5,
    IDT_DPL_1 = 1 << 5,
    IDT_DPL_2 = 2 << 5,
    IDT_DPL_3 = 3 << 5,

    IDT_PRESENT = 1 << 7,
};

struct idt_gate_descriptor idt_encode_descriptor(uint32_t offset, uint16_t segment_selector, uint8_t ist, enum idt_flags type);

void idt_load(struct idt_gate_descriptor table[], uint16_t size);

