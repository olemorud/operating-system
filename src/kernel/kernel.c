#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "interrupts.h"
#include "types.h"
#include "kernel_state.h"
#include "pic.h"

// Future user-space
#include "libc.h"
#include "tty.h"
#include "str.h"

static void user_mode_code(void*)
{
    printf(str_attach("hello from user-space before interrupt :)\n"));
    //__asm__ volatile ("int $0x80");
    //while (1) /* busy loop */;

#if 0
    printf(str_attach("hello from user-space before exception :)\n"));
    /* test division by 0 exception */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdiv-by-zero"
    volatile int a = 5/0;
    (void)a;
    #pragma GCC diagnostic pop

    // should not happen
    printf(str_attach("hello from userspace after interrupt and exception!\n"));
#endif
    
}

static void ring3_mode(segment_t udata_segment, segment_t ucode_segment, func_t callback)
{
    __asm__ volatile (
        "mov %[udata], %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "push %%ax\n"

        "mov %%esp, %%eax\n"
        "push %%eax\n"       // esp
        "pushf\n"            // eflags
        "push %[ucode]\n"
        "push %[callback]\n" // instruction address to return to
        "iret"
        :
        : [udata]    "i"(udata_segment),
          [ucode]    "i"(ucode_segment),
          [callback] "i"(callback)
        : "eax"
    );
}
/*-----------------------------------------------*/

/**
 * Kernel entrypoint
 * =================
 * The kernel entrypoints sets up the GDT, TSS and IDT and moves to ring 3
 */
void kernel_main(void)
{
    __asm__ volatile("cli");


    /* Set up the GDT
	 * ============== */
    kernel.gdt[SEGMENT_NULL] = gdt_encode_entry((struct gdt_entry_content){0});

    /* kernel */
    kernel.gdt[SEGMENT_KERNEL_CODE] = gdt_encode_entry((struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_32BIT
                            | GDT_GRANULARITY_PAGEWISE});

    kernel.gdt[SEGMENT_KERNEL_DATA] = gdt_encode_entry((struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_32BIT
                            | GDT_GRANULARITY_PAGEWISE});

    /* user */
    kernel.gdt[SEGMENT_USER_CODE] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_32BIT
                            | GDT_GRANULARITY_PAGEWISE});

    kernel.gdt[SEGMENT_USER_DATA] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_32BIT
                            | GDT_GRANULARITY_PAGEWISE});

    /* tss */
    kernel.gdt[SEGMENT_TASK_STATE] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = (uint32_t)&kernel.tss,
                     .limit = sizeof(kernel.tss)-1,
                     .access_byte = GDT_ACCESS_ACCESSED
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = 0});

    gdt_load(kernel.gdt,
             sizeof kernel.gdt,
             segment(SEGMENT_KERNEL_DATA, SEGMENT_GDT, 0),
             segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0));

    /* 
	 * Setup the TSS
	 * ============= */
	static uint8_t kernel_stack[KERNEL_STACK_SIZE];
    kernel.tss.ss0 = segment(SEGMENT_KERNEL_DATA, SEGMENT_GDT, 0);
    kernel.tss.esp0 = (uint32_t)kernel_stack;
    tss_load(segment(SEGMENT_TASK_STATE, SEGMENT_GDT, 0));

    /**
	 * Setup the IDT
	 * ============= */
#define m_idt_default(func, type) \
    idt_encode_descriptor( \
            func, \
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0), \
            IDT_DPL_3, \
            type)

#define mtrap(func) m_idt_default(func, IDT_GATE_TYPE_TRAP32);
#define mint(func)  m_idt_default(func, IDT_GATE_TYPE_INTERRUPT32);

	idt_init_stubs(kernel.idt);

    /* Exceptions */
    kernel.idt[IDT_DESC_EXCEPTION_DIVISION_ERROR]           = mtrap(exception_handler_div_by_zero);
    kernel.idt[IDT_DESC_EXCEPTION_DOUBLE_FAULT]             = mtrap(exception_handler_double_fault);
    kernel.idt[IDT_DESC_EXCEPTION_GENERAL_PROTECTION_FAULT] = mtrap(exception_handler_general_protection_fault);

    /* IRQs */
	kernel.idt[IDT_DESC_PIC1 + 0] = mint(irq_handler_0);
	kernel.idt[IDT_DESC_PIC1 + 1] = mint(irq_handler_1);
	kernel.idt[IDT_DESC_PIC1 + 2] = mint(irq_handler_2);
	kernel.idt[IDT_DESC_PIC1 + 3] = mint(irq_handler_3);
	kernel.idt[IDT_DESC_PIC1 + 4] = mint(irq_handler_4);
	kernel.idt[IDT_DESC_PIC1 + 5] = mint(irq_handler_5);
	kernel.idt[IDT_DESC_PIC1 + 6] = mint(irq_handler_6);
	kernel.idt[IDT_DESC_PIC1 + 7] = mint(irq_handler_7);

	kernel.idt[IDT_DESC_PIC2 + 0] = mint(irq_handler_8);
	kernel.idt[IDT_DESC_PIC2 + 1] = mint(irq_handler_9);
	kernel.idt[IDT_DESC_PIC2 + 2] = mint(irq_handler_10);
	kernel.idt[IDT_DESC_PIC2 + 3] = mint(irq_handler_11);
	kernel.idt[IDT_DESC_PIC2 + 4] = mint(irq_handler_12);
	kernel.idt[IDT_DESC_PIC2 + 5] = mint(irq_handler_13);
	kernel.idt[IDT_DESC_PIC2 + 6] = mint(irq_handler_14);
	kernel.idt[IDT_DESC_PIC2 + 7] = mint(irq_handler_15);
    
    /* Interrupts */
    kernel.idt[IDT_DESC_INTERRUPT_SYSCALL] = mint(interrupt_handler_1);
#undef mtrap
#undef mint
#undef m_idt_default

    idt_load(kernel.idt, sizeof kernel.idt);

	/**
	 * PIC setup
	 * =========
	 */

    //irq_set_mask(0xff); /* Disable all IRQs */
    pic8259_remap(IDT_DESC_PIC1, IDT_DESC_PIC2);

    /* enable interrupts */
    __asm__ volatile("sti");

    printf(str_attach("hello from kernel space!\n"));

    /* Finally go to ring 3 */
    ring3_mode(segment(SEGMENT_USER_DATA, SEGMENT_GDT, 3),
               segment(SEGMENT_USER_CODE, SEGMENT_GDT, 3),
               user_mode_code);
}

