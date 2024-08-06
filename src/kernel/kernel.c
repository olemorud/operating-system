#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "tss.h"
#include "idt.h"

// Future user-space
#include "libc.h"
#include "tty.h"

// TOOD: clean up this
typedef void(*func_t)(void*);

static void ring3_mode(uint32_t, uint32_t, uint32_t, uint32_t, func_t);
static void user_mode_code(void*);

void gdt_setup_flat_model()
{
}

/* copied and only slightly modified from an osdev wiki article with poor
   taste, TODO: change
   ----------------------------------------------*/
static void user_mode_code(void*)
{
    printf(str_attach("hello world :)\n"));
}

static void ring3_mode(uint32_t udata_offset, uint32_t udata_rpl, uint32_t ucode_offset, uint32_t ucode_rpl, func_t callback)
{
    const uint32_t udata = udata_offset | udata_rpl;
    const uint32_t ucode = ucode_offset | ucode_rpl;

    __asm__ volatile (
		"mov %[udata], %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"push %%ax\n"

        "mov %%esp, %%eax\n"
        "push %%eax\n"       // current esp
        "pushf\n"            // eflags
        "push %[ucode]\n"
        "push %[callback]\n" // instruction address to return to
        "iret"
        :
        : [udata]    "m"(udata),
          [ucode]    "m"(ucode),
          [callback] "m"(callback)
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

    static struct tss tss = {0};

    enum segment_index : size_t {
        null_segment        = 0,
        kernel_code_segment = 1,
        kernel_data_segment = 2,
        user_code_segment   = 3,
        user_data_segment   = 4,
        task_state_segment  = 5,
        segment_count,
    };

    static struct gdt_table_entry gdt[segment_count];
    _Static_assert(sizeof(gdt) == 0x30);

    gdt[null_segment] = gdt_encode_entry((struct gdt_entry_content){0});

    /* kernel */
    gdt[kernel_code_segment] = gdt_encode_entry((struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    gdt[kernel_data_segment] = gdt_encode_entry((struct gdt_entry_content){
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    /* user */
    gdt[user_code_segment] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    gdt[user_data_segment] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = 0,
                     .limit = GDT_LIMIT_MAX,
                     .access_byte = GDT_ACCESS_RW
                                  | GDT_ACCESS_DESCRIPTOR
                                  | GDT_ACCESS_DPL_3
                                  | GDT_ACCESS_PRESENT,
                     .flags = GDT_SIZE
                            | GDT_GRANULARITY});

    /* tss */
    gdt[task_state_segment] = gdt_encode_entry((struct gdt_entry_content) {
                     .base = (uint32_t)&tss,
                     .limit = sizeof(tss)-1,
                     .access_byte = GDT_ACCESS_ACCESSED
                                  | GDT_ACCESS_EXEC
                                  | GDT_ACCESS_DPL_0
                                  | GDT_ACCESS_PRESENT,
                     .flags = 0});

    gdt_set(sizeof gdt, gdt, 0);

    gdt_reload(kernel_data_segment * sizeof (struct gdt_table_entry),
               kernel_code_segment * sizeof (struct gdt_table_entry));

    /* Setup the TSS */
    tss.ss0 = kernel_data_segment * sizeof (struct gdt_table_entry);
    tss.esp0 = 0; /* TODO: set kernel stack pointer */

    tss_load(task_state_segment * sizeof (struct gdt_table_entry));

	/* Setup the IDT */
	//static struct idt_gate_descriptor idt[256] = {0};

	//struct idt_gate_descriptor idt_encode_descriptor(uint32_t offset, uint16_t segment_selector, uint8_t ist, enum idt_flags type);
	//idt[80] = idt_encode_descriptor(interrupt_handler_1, kernel_code_segment, 0, IDT_GATE_TYPE_INTERUPT_GATE_32);

	//idt_load(idt, sizeof idt);

	/* Finally go to ring 3 */
    ring3_mode(user_data_segment * sizeof (struct gdt_table_entry), 3,
               user_code_segment * sizeof (struct gdt_table_entry), 3,
               user_mode_code);


    __asm__ volatile("sti");
}

