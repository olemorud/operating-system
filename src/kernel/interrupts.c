#include <stdint.h>
#include "libc.h"
#include "tty.h"
#include "interrupts.h"
#include "kernel_state.h"

#define EXCEPTION_DEPTH_MAX 5

struct __attribute__((packed)) interrupt_frame {
    uword_t ip;
    uword_t cs;
    uword_t flags;
    uword_t sp;
    uword_t ss;
};

static void print_interrupt_frame(struct interrupt_frame* f)
{
    printf(str_attach(
           "Interrupt frame:\n"
           "================\n"
           "ip:    {x32}\n"
           "cs:    {x32}\n"
           "flags: {x32}\n"
           "sp:    {x32}\n"
           "ss:    {x32}\n"),
           f->ip,
           f->cs,
           f->flags,
           f->sp,
           f->ss);
}

/**
 * Exceptions
 * ==========
 */
__attribute__((interrupt, noreturn))
void exception_handler_general_protection_fault(struct interrupt_frame* frame, int err)
{
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    //terminal_clear();
    terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printf(str_attach(
        "general protection fault by segment selector {str} :(\n"),
        gdt_segment_index_str[err/8]);
    if (frame != NULL) {
        print_interrupt_frame(frame);
    }
    __asm__ volatile("cli; hlt");
    __builtin_unreachable();
}

__attribute__((interrupt, noreturn))
void exception_handler_double_fault(struct interrupt_frame* frame)
{
    (void)frame;
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    panic(str_attach("double fault :("));
}

__attribute__((interrupt, noreturn))
void exception_handler_div_by_zero(struct interrupt_frame* frame)
{
    (void)frame;
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    panic(str_attach("div by zero occured :("));
}

__attribute__((interrupt, noreturn))
void exception_handler_page_fault(struct interrupt_frame* frame, int err)
{
    (void)frame;
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printf(str_attach(
        "page fault :(, err: 0x{x32}\n"),
        err,
        gdt_segment_index_str[err/8]);
    __asm__ volatile("cli; hlt");
    __builtin_unreachable();
}

__attribute__((interrupt, noreturn))
void exception_default(struct interrupt_frame* frame)
{
    (void)frame;
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    panic(str_attach("non-implemented exception occured"));
    
    kernel.nested_exception_counter = 0;
}

/**
 * Interrupts
 * ==========
 */
__attribute__((interrupt, noreturn))
void interrupt_default(struct interrupt_frame* frame)
{
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    (void)frame;
    panic(str_attach("non-implemented interrupt invoked"));
    
    kernel.nested_exception_counter = 0;
}

__attribute__((interrupt))
void interrupt_handler_1(struct interrupt_frame* frame)
{
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    (void)frame;
    printf(str_attach("interrupt_handler_1 called!\n"));

    kernel.nested_exception_counter = 0;
}

__attribute__((interrupt))
void interrupt_handler_userspace_exit(struct interrupt_frame* frame)
{
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }
    (void)frame;
    printf(str_attach("interrupt_handler_userspace_exit called!\n"));

    kernel.nested_exception_counter = 0;
}


