#pragma once

#include <stdint.h>
#include "str.h"

enum pic8259_port : uint16_t {
    PIC1_COMMAND = 0x20,
    PIC1_DATA    = 0x21,

    PIC2_COMMAND = 0xA0,
    PIC2_DATA    = 0xA1,

    PIC_KEYBOARD = 0x60,
};

static const struct str pic8259_port_str[] = {
    [PIC1_COMMAND        ] = str_attach("PIC1_COMMAND"),
    [PIC1_DATA           ] = str_attach("PIC1_DATA"),
    [PIC2_COMMAND        ] = str_attach("PIC2_COMMAND"),
    [PIC2_DATA           ] = str_attach("PIC2_DATA"),
};


enum irq : uint16_t {
    IRQ_SYSTEM_TIMER          = 0,
    IRQ_KEYBOARD_CONTROLLER   = 1,
    IRQ_SERIAL_COM2           = 3,
    IRQ_SERIAL_COM1           = 4,
    IRQ_LINE_PRINT_TERMINAL_2 = 5,
    IRQ_FLOPPY_CONTROLLER     = 6,
    IRQ_LINE_PRINT_TERMINAL_1 = 7,
    IRQ_RTC_TIMER             = 8,
    IRQ_ACPI                  = 9,
    IRQ_MOUSE_CONTROLLER      = 12,
    IRQ_MATH_COPROCESSOR      = 13,
    IRQ_ATA_CHANNEL_1         = 14,
    IRQ_ATA_CHANNEL_2         = 15,
};

static const struct str irq_str[] = {
    [IRQ_SYSTEM_TIMER         ] = str_attach("IRQ_SYSTEM_TIMER"),
    [IRQ_KEYBOARD_CONTROLLER  ] = str_attach("IRQ_KEYBOARD_CONTROLLER"),
    [IRQ_SERIAL_COM2          ] = str_attach("IRQ_SERIAL_COM2"),
    [IRQ_SERIAL_COM1          ] = str_attach("IRQ_SERIAL_COM1"),
    [IRQ_LINE_PRINT_TERMINAL_2] = str_attach("IRQ_LINE_PRINT_TERMINAL_2"),
    [IRQ_FLOPPY_CONTROLLER    ] = str_attach("IRQ_FLOPPY_CONTROLLER"),
    [IRQ_LINE_PRINT_TERMINAL_1] = str_attach("IRQ_LINE_PRINT_TERMINAL_1"),
    [IRQ_RTC_TIMER            ] = str_attach("IRQ_RTC_TIMER"),
    [IRQ_ACPI                 ] = str_attach("IRQ_ACPI"),
    [IRQ_MOUSE_CONTROLLER     ] = str_attach("IRQ_MOUSE_CONTROLLER"),
    [IRQ_MATH_COPROCESSOR     ] = str_attach("IRQ_MATH_COPROCESSOR"),
    [IRQ_ATA_CHANNEL_1        ] = str_attach("IRQ_ATA_CHANNEL_1"),
    [IRQ_ATA_CHANNEL_2        ] = str_attach("IRQ_ATA_CHANNEL_2"),
};

/**
    Initialization Command Words
    =======================
    From the Intel manuals:
    https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf
    https://web.archive.org/web/20240606163819/https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf
 
    > Whenever a command is issued with A0 = 0 and D4 = 1, this is interpreted
    > as Initialization Command Word 1 (ICW1) CW1 starts the intiitalization
    > sequence during which the following automatically occur:
    > a. The edge sense circuit is reset, which means that following
    >    initialization, an interrupt request (IR) input must make a
    >    low-to-high transistion to generate an interrupt.
    > b. The Interrupt Mask Register is cleared.
    > c. IR7 input is assigned priority 7.
    > d. The slave mode address is set to 7.
    > e. Special Mask Mode is cleared and Status Read is set to IRR
    > f. If IC4 = 0, then all functions selected in ICW4 are set to zero.
    > (Non-Buffered mode*, no Auto-EOI, MCS-80, 85 system).
    > [*]: Master/Slave in ICW4 is only used in the buffered mode
   */
enum icw1 : uint8_t {
    ICW1_ICW4      = 1<<0, /* 1: ICW4 Needed
                              0: ICW4 Not needed */

    ICW1_SINGLE    = 1<<1, /* 1: Single
                              0: Cascade mode */

    ICW1_INTERVAL4 = 1<<2, /* Call Address Interval:
                              1: interval of 4
                              0: interval of 8 */

    ICW1_LEVEL     = 1<<3, /* 1: Level triggered mode
                              0: Edge triggered mode */

    ICW1_INIT      = 1<<4, /* Must be present */
};

enum icw4 : uint8_t {
    ICW4_8086  = 1<<0, /* 1: 8086/88 mode
                          0: MCS-80/85 mode */

    ICW4_AUTO  = 1<<1, /* 1: Auto EOI
                          0: Normal EOI */

    ICW4_BUF1  = 0b11<<2, /* Buffered mode master */

    ICW4_BUF2  = 0b10<<2, /* Buffered mode slave */

    ICW4_SFNM  = 1<<4, /* 1: Special fully nested mode
                          0: Not special fully nested mode */
};

/* OCW2 */
enum ocw2_command : uint8_t {
    OCW2_EOI      = 1<<5,
    OCW2_SPECIFIC = 1<<6,
    OCW2_ROTATE   = 1<<7,

    /* Determines the interrupt level acted upon when the specific flag is set */
    OCW2_IR_LEVEL_0 = 0,
    OCW2_IR_LEVEL_1 = 1,
    OCW2_IR_LEVEL_2 = 2,
    OCW2_IR_LEVEL_3 = 3,
    OCW2_IR_LEVEL_4 = 4,
    OCW2_IR_LEVEL_5 = 5,
    OCW2_IR_LEVEL_6 = 6,
    OCW2_IR_LEVEL_7 = 7,
};

/* OCW3 */
enum ocw3_bits : uint8_t {
    OCW3_BIT_RIS  = (1<<0),
    OCW3_BIT_RR   = (1<<1),
    OCW3_BIT_P    = (1<<2),
    /* bit 3 and 4 must be 0b01 for OCW3 to be selected */
    OCW3_BIT_SMM  = (1<<5),
    OCW3_BIT_ESMM = (1<<6),
};

enum ocw3_command : uint8_t {
    OCW3_READ_ISR           = (1<<3) | OCW3_BIT_RR | OCW3_BIT_RIS,
    OCW3_READ_IRR           = (1<<3) | OCW3_BIT_RR,
    OCW3_POLL               = (1<<3) | OCW3_BIT_P,
    OCW3_RESET_SPECIAL_MASK = (1<<3) | OCW3_BIT_SMM,
    OCW3_SET_SPECIAL_MASK   = (1<<3) | OCW3_BIT_ESMM | OCW3_BIT_SMM,
};

static inline void outb(uint16_t port, uint8_t signal)
{
    __asm__ volatile ("outb %[signal], %[port]"
                    :
                    : [signal] "a"  (signal),
                      [port]   "Nd" (port)
                    : "memory");
#ifdef ADD_IO_WAIT
    /* do an I/O operation on an unused port to wait 1-4 microseconds */
    outb(0x80, 0);
#endif
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

void pic8259_remap(int offset1, int offset2);

void pic8259_set_irq_mask(uint16_t mask);

void pic8259_clear_irq_mask(uint16_t mask);
