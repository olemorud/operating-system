
#pragma once

#include "str.h"
#include "tss.h"
#include "idt.h"
#include "gdt.h"

/*
 * Constants for the GDT
 * */
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

/**
 * Constants for the TSS
 */
constexpr size_t KERNEL_STACK_SIZE = 1024;

/*
 * Constants for IDT descriptors
 * */
static constexpr size_t IDT_EXCEPTION_COUNT = 32;
static constexpr size_t IDT_IRQ_MASTER_COUNT = 8;
static constexpr size_t IDT_IRQ_SLAVE_COUNT = 8;

enum idt_desc_index : size_t {
    /* Exceptions */
    IDT_DESC_EXCEPTION_DIVISION_ERROR              = 0,
    IDT_DESC_EXCEPTION_DEBUG                       = 1,
    IDT_DESC_EXCEPTION_NON_MASKABLE                = 2,
    IDT_DESC_EXCEPTION_BREAKPOINT                  = 3,
    IDT_DESC_EXCEPTION_OVERFLOW                    = 4,
    IDT_DESC_EXCEPTION_BOUND_RANGE_EXCEEDED        = 5,
    IDT_DESC_EXCEPTION_INVALID_OPCODE              = 6,
    IDT_DESC_EXCEPTION_DEVICE_NOT_AVAILABLE        = 7,
    IDT_DESC_EXCEPTION_DOUBLE_FAULT                = 8,
    IDT_DESC_EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN = 9,
    IDT_DESC_EXCEPTION_INVALID_TSS                 = 10,
    IDT_DESC_EXCEPTION_SEGMENT_NOT_PRESENT         = 11,
    IDT_DESC_EXCEPTION_STACK_SEGMENTATION_FAULT    = 12,
    IDT_DESC_EXCEPTION_GENERAL_PROTECTION_FAULT    = 13,
    IDT_DESC_EXCEPTION_PAGE_FAULT                  = 14,
    /* 15 is reserved */
    IDT_DESC_EXCEPTION_X87_FLOATING_POINT          = 16,
    IDT_DESC_EXCEPTION_ALIGNMENT_CHECK             = 17,
    IDT_DESC_EXCEPTION_MACHINE_CHECK               = 18,
    IDT_DESC_EXCEPTION_SIMD_FLOATING_POINT         = 19,
    IDT_DESC_EXCEPTION_VIRTUALIZATION              = 20,
    IDT_DESC_EXCEPTION_CONTROL_PROTECTION          = 21,
    /* 21 to 27 are reserved */
    IDT_DESC_EXCEPTION_HYPERVISOR_INJECTION        = 28,
    IDT_DESC_EXCEPTION_VMM_COMMUNICATION           = 29,
    IDT_DESC_EXCEPTION_SECURITY                    = 30,
    /* 31 is reserved */

    /* IRQ offsets */
    IDT_DESC_PIC1 = 32,
    IDT_DESC_PIC2 = IDT_DESC_PIC1 + 8,

    IDT_DESC_INTERRUPT_SYSCALL = 128,

    IDT_DESC_COUNT = 256,
};
extern const struct str idt_desc_index_str[IDT_DESC_COUNT]; // reverse lookup enum -> str

/**
 * The global kernel state object
 */
struct kernel_state {
    struct tss tss;
    struct gdt_table_entry gdt[SEGMENT_COUNT];
    struct idt_gate_descriptor idt[IDT_DESC_COUNT];
	int nested_exception_counter;
};
extern struct kernel_state kernel;
