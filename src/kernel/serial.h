#pragma once

#include "str.h"

/*
 * TODO: consolidate memory mapped addresses into one header file, maybe per
 * architecture
 * */

//static uint8_t* SERIAL_COM1 = (uint8_t*)0x03F8;
//static uint8_t* SERIAL_COM2 = (uint8_t*)0x02F8;

enum serial_port : uint16_t {
    SERIAL_COM1 = 0x03F8,
    SERIAL_COM2 = 0x02F8,
};

enum serial_port_offsets : uint8_t {
    SERIAL_DATA_REGISTER    = 0,
    SERIAL_INTERRUPT        = 1,

    SERIAL_DLAB_LSB         = 0, /* when dlab is set this is the least
                                    significant byte for the divisor*/
    SERIAL_DLAB_MSB         = 1, /* ... and this is the most significant byte*/

    SERIAL_INTERRUPT_IDENTIFICATION = 2,
    SERIAL_FIFO_CONTROL     = 2,
    SERIAL_LINE_CONTROL     = 3,
    SERIAL_MODEM_CONTROL    = 4,
    SERIAL_LINE_STATUS      = 5,
    SERIAL_MODEM_STATUS     = 6,
    SERIAL_SCRATCH_REGISTER = 7,
};

enum serial_line_control : uint8_t {
        SERIAL_LINE_DATA_5_BIT   = 0b00<<0,
        SERIAL_LINE_DATA_6_BIT   = 0b01<<0,
        SERIAL_LINE_DATA_7_BIT   = 0b10<<0,
        SERIAL_LINE_DATA_8_BIT   = 0b11<<0,

        SERIAL_LINE_STOP_1       = 0<<2,
        SERIAL_LINE_STOP_2       = 1<<2,

        SERIAL_LINE_PARITY_NONE  = 0b000<<3,
        SERIAL_LINE_PARITY_ODD   = 0b001<<3,
        SERIAL_LINE_PARITY_EVEN  = 0b011<<3,
        SERIAL_LINE_PARITY_MARK  = 0b101<<3,
        SERIAL_LINE_PARITY_SPACE = 0b111<<3,

        SERIAL_LINE_BREAK_ENABLE = 1<<6,

        SERIAL_LINE_DLAB         = 1<<7,
};

enum serial_interrupt_control : uint8_t {
    SERIAL_INTERRUPT_RECEIVED_DATA_AVAILABLE            = 1<<0,
    SERIAL_INTERRUPT_TRANSMITTER_HOLDING_REGISTER_EMPTY = 1<<1,
    SERIAL_INTERRUPT_RECEIVER_LINE_STATUS               = 1<<2,
    SERIAL_INTERRUPT_MODEM_STATUS                       = 1<<3,
};

enum serial_fifo_control : uint8_t {
    SERIAL_FIFO_ENABLE            = 1<<0,
    SERIAL_FIFO_CLEAR_RECEIVE     = 1<<1,
    SERIAL_FIFO_CLEAR_TRANSMIT    = 1<<2,
    SERIAL_FIFO_DMA_MODE          = 1<<3,
    SERIAL_FIFO_INTERRUPT_1_BYTE  = 0<<6,
    SERIAL_FIFO_INTERRUPT_4_BYTE  = 1<<6,
    SERIAL_FIFO_INTERRUPT_8_BYTE  = 2<<6,
    SERIAL_FIFO_INTERRUPT_14_BYTE = 3<<6,
};

enum serial_modem_control : uint8_t {
    SERIAL_MODEM_DATA_READY      = 1<<0,
    SERIAL_MODEM_REQUEST_TO_SEND = 1<<1,
    SERIAL_MODEM_OUT_1           = 1<<2,
    SERIAL_MODEM_OUT_2           = 1<<3,
    SERIAL_MODEM_ENABLE_IRQ      = 1<<3, /* alias for out_2 */
    SERIAL_MODEM_LOOP            = 1<<4,
    SERIAL_MODEM_UNUSED_0        = 1<<5,
    SERIAL_MODEM_UNUSED_1        = 1<<6,
    SERIAL_MODEM_UNUSED_3        = 1<<7,
};

enum serial_line_status: uint8_t {
    SERIAL_LINE_STATUS_DATA_READY        = 1<<0,
    SERIAL_LINE_STATUS_OVERRUN_ERROR     = 1<<1,
    SERIAL_LINE_STATUS_PARITY_ERROR      = 1<<2,
    SERIAL_LINE_STATUS_FRAMING_ERROR     = 1<<3,
    SERIAL_LINE_STATUS_BREAK_INDICATOR   = 1<<4,
    SERIAL_LINE_STATUS_TRANSMISSION_BUF_EMPTY              = 1<<5,
    SERIAL_LINE_STATUS_TRANSMITTER_EMPTY = 1<<6,
    SERIAL_LINE_STATUS_IMPENDING_ERROR   = 1<<7,
};

enum serial_modem_status : uint8_t {
    SERIAL_MODEM_STATUS_DCTS = 1<<0,
    SERIAL_MODEM_STATUS_DDSR = 1<<1,
    SERIAL_MODEM_STATUS_TERI = 1<<2,
    SERIAL_MODEM_STATUS_DDCD = 1<<3,
    SERIAL_MODEM_STATUS_CTS  = 1<<4,
    SERIAL_MODEM_STATUS_DSR  = 1<<5,
    SERIAL_MODEM_STATUS_RI   = 1<<6,
    SERIAL_MODEM_STATUS_DCD  = 1<<7,
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
    __asm__ volatile ( "inb %[port], %[ret]"
                   : [ret]  "=a"(ret)
                   : [port] "Nd"(port)
                   : "memory");
    return ret;
}

static int serial_init(enum serial_port serial, uint16_t baud_divisor)
{
    outb(serial + SERIAL_INTERRUPT,    SERIAL_LINE_DLAB);
    outb(serial + SERIAL_LINE_CONTROL, SERIAL_LINE_DLAB);
    outb(serial + SERIAL_DLAB_LSB,     baud_divisor & 0x0F);
    outb(serial + SERIAL_DLAB_MSB,     (baud_divisor & 0xF0) >> 8);
    outb(serial + SERIAL_LINE_CONTROL, SERIAL_LINE_DATA_8_BIT);

    outb(serial + SERIAL_FIFO_CONTROL, 0
        | SERIAL_FIFO_ENABLE
        | SERIAL_FIFO_INTERRUPT_14_BYTE
        | SERIAL_FIFO_CLEAR_RECEIVE);

    outb(serial + SERIAL_MODEM_CONTROL, 0
        | SERIAL_MODEM_DATA_READY
        | SERIAL_MODEM_REQUEST_TO_SEND
        | SERIAL_MODEM_ENABLE_IRQ);

    outb(serial + SERIAL_MODEM_CONTROL, 0
        | SERIAL_MODEM_REQUEST_TO_SEND
        | SERIAL_MODEM_OUT_1
        | SERIAL_MODEM_OUT_2
        | SERIAL_MODEM_LOOP);

    constexpr uint8_t ping = 0xAE;

    outb(serial + SERIAL_DATA_REGISTER, ping);

    if (inb(serial + SERIAL_DATA_REGISTER) != ping) {
        return -1;
    }

    outb(serial + SERIAL_MODEM_CONTROL, 0
        | SERIAL_MODEM_REQUEST_TO_SEND
        | SERIAL_MODEM_DATA_READY
        | SERIAL_MODEM_OUT_1
        | SERIAL_MODEM_OUT_2
    );

    return 0;
}

static int serial_is_transmit_empty(enum serial_port port) {
    return inb(port + SERIAL_LINE_STATUS)
         & SERIAL_LINE_STATUS_TRANSMISSION_BUF_EMPTY;
}

static void serial_putchar(enum serial_port port, char ch)
{
    while (serial_is_transmit_empty(port) == 0)
        /* spin lock */;
    
    outb(port, ch);
}

static void serial_write(enum serial_port port, struct str s)
{
    for (size_t i = 0; i < s.len; i++) {
        serial_putchar(port, s.data[i]);
    }
}


