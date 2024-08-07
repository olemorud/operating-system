#pragma once

#include "types.h"

struct __attribute__((packed)) interrupt_frame {
    uword_t ip;
    uword_t cs;
    uword_t flags;
    uword_t sp;
    uword_t ss;
};

__attribute__((interrupt, noreturn))
void exception_handler_div_by_zero(struct interrupt_frame* frame);

__attribute__((interrupt, noreturn))
void exception_handler_general_protection_fault(struct interrupt_frame* frame, int err);

__attribute__((interrupt, noreturn))
void exception_handler_double_fault(struct interrupt_frame* frame);

__attribute__((interrupt, noreturn))
void exception_default(struct interrupt_frame* frame);

__attribute__((interrupt, noreturn))
void interrupt_default(struct interrupt_frame* frame);

__attribute__((interrupt))
void interrupt_handler_1(struct interrupt_frame* frame);

__attribute__((interrupt))
void interrupt_handler_userspace_exit(struct interrupt_frame* frame);

