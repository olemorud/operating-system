
#pragma once

#include "str.h"
#include "tss.h"
#include "idt.h"
#include "gdt.h"

enum gdt_segment_index : size_t {
    SEGMENT_NULL,
    SEGMENT_KERNEL_CODE,
    SEGMENT_KERNEL_DATA,
    SEGMENT_USER_CODE,
    SEGMENT_USER_DATA,
    SEGMENT_TASK_STATE,
    SEGMENT_COUNT,
};
extern const struct str gdt_segment_index_str[SEGMENT_COUNT]; // reverse lookup enum -> str

enum idt_index : size_t {
    IDT_COUNT = 256
};
extern const struct str idt_index_str[IDT_COUNT]; // reverse lookup enum -> str

constexpr size_t BACKTRACE_MAX = 256;

struct kernel_state {
    struct tss tss;
    struct gdt_table_entry gdt[SEGMENT_COUNT];
    struct idt_gate_descriptor idt[IDT_COUNT];
	int nested_exception_counter;
};

extern struct kernel_state kernel;
