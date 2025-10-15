#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "file_system.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "kernel_state.h"
#include "page.h"
#include "pic.h"
#include "serial.h"
#include "tss.h"
#include "types.h"

// Future user-space
#include "libc.h"
#include "tty.h"
#include "str.h"
#include "bitmap.h"
#include "syscall.h"

__attribute__((section(".userland-text")))
void user_mode_code(void*)
{
    const char msg[] = "hello from ring 3\n";
    syscall(SYSCALL_PRINT, &str_attach(msg), 0, 0);

    volatile uint32_t a = *(uint32_t*)0;
#if 0
    syscall(SYSCALL_PRINT, &str_attach("trying to divide by zero :)\n"), 0, 0);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdiv-by-zero"
    volatile int a = 5/0;
    (void)a;
    #pragma GCC diagnostic pop

    // should not happen
    syscall(SYSCALL_PRINT, &str_attach("hello from userspace after interrupt and exception!\n"), 0, 0);
#endif

    syscall(SYSCALL_EXIT, 0, 0, 0);

    //#pragma GCC diagnostic push
    //#pragma GCC diagnostic ignored "-Wunused-value"
    //*(uint32_t*)0;
    //#pragma GCC diagnostic pop
}

typedef uint32_t pde_t;

struct proc {
    uint32_t (*page_directory)[1024];
    uint8_t* kernel_stack;
    char     name[16];
    uint32_t mem_size;
    
    // TODO
    //struct   trapframe *tf;
    //struct context *context;     // swtch() here to run process
    //void *chan;                  // If non-zero, sleeping on chan
    //int killed;                  // If non-zero, have been killed
    //struct file *ofile[NOFILE];  // Open files
    //struct inode *cwd;           // Current directory
};


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
        "push %%eax\n"
        "pushf\n"
        "push %[ucode]\n"
        "push %[callback]\n"
        "iret"
        :
        : [udata]    "m"(udata_segment),
          [ucode]    "m"(ucode_segment),
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


    /* Set up the serial port
     * ======================
     */
    if (serial_init(SERIAL_COM1, 3) != 0) {
        panic(str_attach("serial_init failed"));
    }

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
    memset(&kernel.tss, 0, sizeof kernel.tss);
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
    kernel.idt[IDT_DESC_EXCEPTION_PAGE_FAULT]               = mtrap(exception_handler_page_fault);

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
    kernel.idt[IDT_DESC_INTERRUPT_SYSCALL] = mint(interrupt_handler_syscall);
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

    /*
     * File system setup
     * =================
     * */
    printf(str_attach("Testing file system...\n"));
    {
        int ok = test_file_system();
        if (ok != 0) {
            panic(str_attach("test_file_system() returned non-zero value"));
        }
    }

    /**
     * Paging setup
     * ============
     * We align by (1<<12) == 4096 because page directory and page table
     * entries store addresses from bit 12-31
     *
     * For now give user access to pages to avoid a page fault, since it's not
     * implemented properly
     */
    printf(str_attach("setting up paging...\n"));
    printf(str_attach("kernel end: {bin}\n"), kernel_memory_end);

    static uint32_t page_directory[1024]    __attribute__((aligned(4096)));
    static uint32_t page_table_kernel[1024] __attribute__((aligned(4096)));
    static uint32_t page_table_user[1024]   __attribute__((aligned(4096)));
    _Static_assert(((uint32_t)page_directory    & 0xfff) == 0);
    _Static_assert(((uint32_t)page_table_kernel & 0xfff) == 0);
    _Static_assert(((uint32_t)page_table_user   & 0xfff) == 0);

    for (size_t i = 0; i < sizeof page_directory / sizeof *page_directory; i++) {
        page_directory[i] = PDE_WRITE; /* no present bit */
    }
    page_directory[sizeof page_directory / sizeof *page_directory - 1] = (uint32_t)page_directory | PDE_WRITE | PDE_PRESENT;

    for (size_t i = 0; i < sizeof page_table_kernel / sizeof *page_table_kernel; i++) {
        page_table_kernel[i] = (i<<12) | PTE_WRITE | PTE_PRESENT;
    }
    page_directory[0] = ((uint32_t)page_table_kernel) | PDE_WRITE | PDE_PRESENT | PDE_USER_ACCESS;

    for (size_t i = 0; i < sizeof page_table_user / sizeof *page_table_user; i++) {
        page_table_user[i] = ((uint32_t)kernel_memory_end + (i<<12)) | PTE_WRITE | PTE_PRESENT | PTE_USER;
    }
    page_directory[1] = ((uint32_t)page_table_user) | PDE_WRITE | PDE_PRESENT | PDE_USER_ACCESS;

    cr3_set((uint32_t)page_directory);
    cr0_flags_set(CR0_PAGING | CR0_PROTECTED_MODE);

    printf(str_attach("done!\n"));

    printf(str_attach("starting code in ring 3...\n"));

    /* Load userspace binary to memory */
    
    printf(str_attach("address of userspace code: {x32} = {addr}\n"), user_mode_code, user_mode_code);

    printf(str_attach("im going ring 3 mode\n"));

    //new_proc();

    /* Finally go to ring 3 */
    ring3_mode(segment(SEGMENT_USER_DATA, SEGMENT_GDT, 3),
               segment(SEGMENT_USER_CODE, SEGMENT_GDT, 3),
               user_mode_code);

    printf(str_attach("back to kernel mode...\n"));

    __asm__ volatile ("hlt");
}
