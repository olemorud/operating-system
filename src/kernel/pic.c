
#include "pic.h"

void pic8259_remap(int offset1, int offset2)
{
    /* ICW1: restart PIC1 and PIC2 with
	   - icw4
	   - cascade
	   - 8 bit interval
	   - edge triggered mode
	   */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    /* ICW2: Send new offsets */
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    /* ICW2 and ICW3: Setup cascading */
    outb(PIC1_DATA, 1<<2); /* tell master PIC that IR input 2 has a slave */
    outb(PIC2_DATA, 2);    /* set ID of slave PIC */
    
    /* ICW4: 8086/88 mode */
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
}

static inline void irq_set(enum pic8259_port port, uint8_t mask)
{
    outb(port, inb(port) | mask);
}

static inline void irq_clear(enum pic8259_port port, uint8_t mask)
{
    outb(port, inb(port) & ~(mask));
}

void pic8259_set_irq_mask(uint16_t mask)
{
    const uint8_t pic1_mask = mask & 0xF;
    mask >>= 8;
    const uint8_t pic2_mask = mask & 0xF;

    if (pic1_mask) {
        irq_set(PIC1_DATA, pic1_mask);
    }
    if (pic2_mask) {
        irq_set(PIC2_DATA, pic2_mask);
    }
}

void pic8259_clear_irq_mask(uint16_t mask)
{
    const uint8_t pic1_mask = mask & 0xF;
    mask >>= 8;
    const uint8_t pic2_mask = mask & 0xF;

    if (pic1_mask) {
        irq_clear(PIC1_DATA, pic1_mask);
    }
    if (pic2_mask) {
        irq_clear(PIC2_DATA, pic2_mask);
    }
}

/**
 * https://wiki.osdev.org/8259_PIC#ISR_and_IRR
 * ===========================================
 * */
static uint16_t get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
       represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic8259_get_irr(void)
{
    return get_irq_reg(OCW3_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic8259_get_isr(void)
{
    return get_irq_reg(OCW3_READ_ISR);
}

/* =========================================== */
