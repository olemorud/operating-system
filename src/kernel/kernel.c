#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "interrupts.h"
#include "types.h"
#include "kernel_state.h"

// Future user-space
#include "libc.h"
#include "tty.h"
#include "str.h"

static void pic_disable()
{
    __asm__ volatile ("outb %0, %1"
            :
            : "a"((uint8_t)0xff), "Nd"((uint16_t)0x21)
            : "memory");
    __asm__ volatile ("outb %0, %1"
            :
            : "a"((uint8_t)0xff), "Nd"((uint16_t)0xA1)
            : "memory");
}

static void user_mode_code(void*)
{
    printf(str_attach("hello from user-space before interrupt :)\n"));
    __asm__ volatile ("int $0x80");
    __asm__ volatile ("int $0x81");
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

    _Static_assert(sizeof(kernel.gdt) == 0x30);

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

    /* Setup the TSS */
    memset(&kernel.tss, 0, sizeof kernel.tss);
    kernel.tss.ss0 = segment(SEGMENT_KERNEL_DATA, SEGMENT_GDT, 0);
#if 1
    static uint8_t kernel_stack[1024];
    kernel.tss.esp0 = (uint32_t)kernel_stack;
#else
    kernel.tss.esp0 = 0;
#endif

#if 1
    /* Setup the IDT */
    for (size_t i = 0; i < 32; i++) {
        kernel.idt[i] = idt_encode_descriptor(
               exception_default,
               segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
               IDT_DPL_3,
               IDT_GATE_TYPE_TRAP32);
    }
    for (size_t i = 32; i < IDT_COUNT; i++) {
        kernel.idt[i] = idt_encode_descriptor(
               interrupt_default,
               segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
               IDT_DPL_3,
               IDT_GATE_TYPE_INTERRUPT32);
    }

    kernel.idt[0x0] = idt_encode_descriptor(
            exception_handler_div_by_zero,
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
            IDT_DPL_3,
            IDT_GATE_TYPE_TRAP32
    );

    kernel.idt[0x8] = idt_encode_descriptor(
            exception_handler_double_fault,
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
            IDT_DPL_3,
            IDT_GATE_TYPE_TRAP32
    );

    kernel.idt[0xD] = idt_encode_descriptor(
            exception_handler_general_protection_fault,
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
            IDT_DPL_3,
            IDT_GATE_TYPE_TRAP32
    );

    // Interrupts
    kernel.idt[0x80] = idt_encode_descriptor(
            interrupt_handler_1,
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
            IDT_DPL_3,
            IDT_GATE_TYPE_INTERRUPT32
    );

    kernel.idt[0x81] = idt_encode_descriptor(
            interrupt_handler_userspace_exit,
            segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0),
            IDT_DPL_3,
            IDT_GATE_TYPE_INTERRUPT32
    );
#endif

    /* Flush the gdt and tss */
    gdt_load(kernel.gdt,
             sizeof kernel.gdt,
             segment(SEGMENT_KERNEL_DATA, SEGMENT_GDT, 0),
             segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0));
    tss_load(segment(SEGMENT_TASK_STATE, SEGMENT_GDT, 0));
    idt_load(kernel.idt, sizeof kernel.idt);

    /* enable interrupts */
    __asm__ volatile("sti");

    /* Finally go to ring 3 */
    printf(str_attach("hello from kernel space!\n"));

    pic_disable(); // temporary duct-tape 

    ring3_mode(segment(SEGMENT_USER_DATA, SEGMENT_GDT, 3),
               segment(SEGMENT_USER_CODE, SEGMENT_GDT, 3),
               user_mode_code);
}

