#include <stdint.h>
#include "libc.h"
#include "tty.h"
#include "interrupts.h"
#include "kernel_state.h"

#include "pic.h"

#define EXCEPTION_DEPTH_MAX 3

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

/* not an interrupt/exception, but called by exception stubs */
__attribute__((noreturn))
static void panic_exception_not_implemented(struct interrupt_frame* frame, int exception_no)
{
    terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printf(str_attach("non-implemented exception {i32} occurred\n"), exception_no);
	struct str name = idt_desc_index_str[exception_no];
	if (name.len != 0) {
		printf(str_attach("exception name: {str})\n"), name);
	}
    if (frame != NULL) {
        print_interrupt_frame(frame);
    } else {
        printf(str_attach("(no interrupt frame)n"));
    }
    __asm__ volatile("cli; hlt");
    __builtin_unreachable();
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

/*
 * IRQs
 * ====
 * */
static void irq_stub(struct interrupt_frame* frame, int line)
{
	(void)frame;
	(void)line;
    if (kernel.nested_exception_counter++ > EXCEPTION_DEPTH_MAX) {
        panic(str_attach("fatal: too many nested exceptions\n"));
    }

    //printf(str_attach("irq handler {i32} called!\n"), line);
	outb(PIC1_COMMAND, OCW2_EOI);
    kernel.nested_exception_counter = 0;
}

__attribute__((interrupt)) void irq_handler_0(struct interrupt_frame* frame)  { irq_stub(frame, 0); }

/* IRQ1 - keyboard controller */
#include "ps2-keyboard.h"
__attribute__((interrupt)) void irq_handler_1(struct interrupt_frame* frame) 
{
	/* TODO: move keyboard logic to a separate compilation unit */
	(void)frame;
	uint8_t key = inb(PIC_KEYBOARD);

	bool released = key & KEY_RELEASED;

	printf(str_attach("key {str}\n"), ps2_key_str[key]);
	outb(PIC1_COMMAND, OCW2_EOI);
}

__attribute__((interrupt)) void irq_handler_2(struct interrupt_frame* frame)  { irq_stub(frame, 2); }
__attribute__((interrupt)) void irq_handler_3(struct interrupt_frame* frame)  { irq_stub(frame, 3); }
__attribute__((interrupt)) void irq_handler_4(struct interrupt_frame* frame)  { irq_stub(frame, 4); }
__attribute__((interrupt)) void irq_handler_5(struct interrupt_frame* frame)  { irq_stub(frame, 5); }
__attribute__((interrupt)) void irq_handler_6(struct interrupt_frame* frame)  { irq_stub(frame, 6); }
__attribute__((interrupt)) void irq_handler_7(struct interrupt_frame* frame)  { irq_stub(frame, 7); }
__attribute__((interrupt)) void irq_handler_8(struct interrupt_frame* frame)  { irq_stub(frame, 8); }
__attribute__((interrupt)) void irq_handler_9(struct interrupt_frame* frame)  { irq_stub(frame, 9); }
__attribute__((interrupt)) void irq_handler_10(struct interrupt_frame* frame) { irq_stub(frame, 10); }
__attribute__((interrupt)) void irq_handler_11(struct interrupt_frame* frame) { irq_stub(frame, 11); }
__attribute__((interrupt)) void irq_handler_12(struct interrupt_frame* frame) { irq_stub(frame, 12); }
__attribute__((interrupt)) void irq_handler_13(struct interrupt_frame* frame) { irq_stub(frame, 13); }
__attribute__((interrupt)) void irq_handler_14(struct interrupt_frame* frame) { irq_stub(frame, 14); }
__attribute__((interrupt)) void irq_handler_15(struct interrupt_frame* frame) { irq_stub(frame, 15); }


/**
 * Exception Stubs
 * ===============
 */

#define DEFINE_EXCEPTION_STUB(n)                          \
    __attribute__((interrupt, noreturn))                  \
    void EXCEPTION_STUB(n)(struct interrupt_frame* frame) \
    {                                                     \
        panic_exception_not_implemented(frame, n);        \
    }

DEFINE_EXCEPTION_STUB(0)
DEFINE_EXCEPTION_STUB(1)
DEFINE_EXCEPTION_STUB(2)
DEFINE_EXCEPTION_STUB(3)
DEFINE_EXCEPTION_STUB(4)
DEFINE_EXCEPTION_STUB(5)
DEFINE_EXCEPTION_STUB(6)
DEFINE_EXCEPTION_STUB(7)
DEFINE_EXCEPTION_STUB(8)
DEFINE_EXCEPTION_STUB(9)
DEFINE_EXCEPTION_STUB(10)
DEFINE_EXCEPTION_STUB(11)
DEFINE_EXCEPTION_STUB(12)
DEFINE_EXCEPTION_STUB(13)
DEFINE_EXCEPTION_STUB(14)
DEFINE_EXCEPTION_STUB(15)
DEFINE_EXCEPTION_STUB(16)
DEFINE_EXCEPTION_STUB(17)
DEFINE_EXCEPTION_STUB(18)
DEFINE_EXCEPTION_STUB(19)
DEFINE_EXCEPTION_STUB(20)
DEFINE_EXCEPTION_STUB(21)
DEFINE_EXCEPTION_STUB(22)
DEFINE_EXCEPTION_STUB(23)
DEFINE_EXCEPTION_STUB(24)
DEFINE_EXCEPTION_STUB(25)
DEFINE_EXCEPTION_STUB(26)
DEFINE_EXCEPTION_STUB(27)
DEFINE_EXCEPTION_STUB(28)
DEFINE_EXCEPTION_STUB(29)
DEFINE_EXCEPTION_STUB(30)
DEFINE_EXCEPTION_STUB(31)
DEFINE_EXCEPTION_STUB(32)
DEFINE_EXCEPTION_STUB(33)
DEFINE_EXCEPTION_STUB(34)
DEFINE_EXCEPTION_STUB(35)
DEFINE_EXCEPTION_STUB(36)
DEFINE_EXCEPTION_STUB(37)
DEFINE_EXCEPTION_STUB(38)
DEFINE_EXCEPTION_STUB(39)
DEFINE_EXCEPTION_STUB(40)
DEFINE_EXCEPTION_STUB(41)
DEFINE_EXCEPTION_STUB(42)
DEFINE_EXCEPTION_STUB(43)
DEFINE_EXCEPTION_STUB(44)
DEFINE_EXCEPTION_STUB(45)
DEFINE_EXCEPTION_STUB(46)
DEFINE_EXCEPTION_STUB(47)
DEFINE_EXCEPTION_STUB(48)
DEFINE_EXCEPTION_STUB(49)
DEFINE_EXCEPTION_STUB(50)
DEFINE_EXCEPTION_STUB(51)
DEFINE_EXCEPTION_STUB(52)
DEFINE_EXCEPTION_STUB(53)
DEFINE_EXCEPTION_STUB(54)
DEFINE_EXCEPTION_STUB(55)
DEFINE_EXCEPTION_STUB(56)
DEFINE_EXCEPTION_STUB(57)
DEFINE_EXCEPTION_STUB(58)
DEFINE_EXCEPTION_STUB(59)
DEFINE_EXCEPTION_STUB(60)
DEFINE_EXCEPTION_STUB(61)
DEFINE_EXCEPTION_STUB(62)
DEFINE_EXCEPTION_STUB(63)
DEFINE_EXCEPTION_STUB(64)
DEFINE_EXCEPTION_STUB(65)
DEFINE_EXCEPTION_STUB(66)
DEFINE_EXCEPTION_STUB(67)
DEFINE_EXCEPTION_STUB(68)
DEFINE_EXCEPTION_STUB(69)
DEFINE_EXCEPTION_STUB(70)
DEFINE_EXCEPTION_STUB(71)
DEFINE_EXCEPTION_STUB(72)
DEFINE_EXCEPTION_STUB(73)
DEFINE_EXCEPTION_STUB(74)
DEFINE_EXCEPTION_STUB(75)
DEFINE_EXCEPTION_STUB(76)
DEFINE_EXCEPTION_STUB(77)
DEFINE_EXCEPTION_STUB(78)
DEFINE_EXCEPTION_STUB(79)
DEFINE_EXCEPTION_STUB(80)
DEFINE_EXCEPTION_STUB(81)
DEFINE_EXCEPTION_STUB(82)
DEFINE_EXCEPTION_STUB(83)
DEFINE_EXCEPTION_STUB(84)
DEFINE_EXCEPTION_STUB(85)
DEFINE_EXCEPTION_STUB(86)
DEFINE_EXCEPTION_STUB(87)
DEFINE_EXCEPTION_STUB(88)
DEFINE_EXCEPTION_STUB(89)
DEFINE_EXCEPTION_STUB(90)
DEFINE_EXCEPTION_STUB(91)
DEFINE_EXCEPTION_STUB(92)
DEFINE_EXCEPTION_STUB(93)
DEFINE_EXCEPTION_STUB(94)
DEFINE_EXCEPTION_STUB(95)
DEFINE_EXCEPTION_STUB(96)
DEFINE_EXCEPTION_STUB(97)
DEFINE_EXCEPTION_STUB(98)
DEFINE_EXCEPTION_STUB(99)
DEFINE_EXCEPTION_STUB(100)
DEFINE_EXCEPTION_STUB(101)
DEFINE_EXCEPTION_STUB(102)
DEFINE_EXCEPTION_STUB(103)
DEFINE_EXCEPTION_STUB(104)
DEFINE_EXCEPTION_STUB(105)
DEFINE_EXCEPTION_STUB(106)
DEFINE_EXCEPTION_STUB(107)
DEFINE_EXCEPTION_STUB(108)
DEFINE_EXCEPTION_STUB(109)
DEFINE_EXCEPTION_STUB(110)
DEFINE_EXCEPTION_STUB(111)
DEFINE_EXCEPTION_STUB(112)
DEFINE_EXCEPTION_STUB(113)
DEFINE_EXCEPTION_STUB(114)
DEFINE_EXCEPTION_STUB(115)
DEFINE_EXCEPTION_STUB(116)
DEFINE_EXCEPTION_STUB(117)
DEFINE_EXCEPTION_STUB(118)
DEFINE_EXCEPTION_STUB(119)
DEFINE_EXCEPTION_STUB(120)
DEFINE_EXCEPTION_STUB(121)
DEFINE_EXCEPTION_STUB(122)
DEFINE_EXCEPTION_STUB(123)
DEFINE_EXCEPTION_STUB(124)
DEFINE_EXCEPTION_STUB(125)
DEFINE_EXCEPTION_STUB(126)
DEFINE_EXCEPTION_STUB(127)
DEFINE_EXCEPTION_STUB(128)
DEFINE_EXCEPTION_STUB(129)
DEFINE_EXCEPTION_STUB(130)
DEFINE_EXCEPTION_STUB(131)
DEFINE_EXCEPTION_STUB(132)
DEFINE_EXCEPTION_STUB(133)
DEFINE_EXCEPTION_STUB(134)
DEFINE_EXCEPTION_STUB(135)
DEFINE_EXCEPTION_STUB(136)
DEFINE_EXCEPTION_STUB(137)
DEFINE_EXCEPTION_STUB(138)
DEFINE_EXCEPTION_STUB(139)
DEFINE_EXCEPTION_STUB(140)
DEFINE_EXCEPTION_STUB(141)
DEFINE_EXCEPTION_STUB(142)
DEFINE_EXCEPTION_STUB(143)
DEFINE_EXCEPTION_STUB(144)
DEFINE_EXCEPTION_STUB(145)
DEFINE_EXCEPTION_STUB(146)
DEFINE_EXCEPTION_STUB(147)
DEFINE_EXCEPTION_STUB(148)
DEFINE_EXCEPTION_STUB(149)
DEFINE_EXCEPTION_STUB(150)
DEFINE_EXCEPTION_STUB(151)
DEFINE_EXCEPTION_STUB(152)
DEFINE_EXCEPTION_STUB(153)
DEFINE_EXCEPTION_STUB(154)
DEFINE_EXCEPTION_STUB(155)
DEFINE_EXCEPTION_STUB(156)
DEFINE_EXCEPTION_STUB(157)
DEFINE_EXCEPTION_STUB(158)
DEFINE_EXCEPTION_STUB(159)
DEFINE_EXCEPTION_STUB(160)
DEFINE_EXCEPTION_STUB(161)
DEFINE_EXCEPTION_STUB(162)
DEFINE_EXCEPTION_STUB(163)
DEFINE_EXCEPTION_STUB(164)
DEFINE_EXCEPTION_STUB(165)
DEFINE_EXCEPTION_STUB(166)
DEFINE_EXCEPTION_STUB(167)
DEFINE_EXCEPTION_STUB(168)
DEFINE_EXCEPTION_STUB(169)
DEFINE_EXCEPTION_STUB(170)
DEFINE_EXCEPTION_STUB(171)
DEFINE_EXCEPTION_STUB(172)
DEFINE_EXCEPTION_STUB(173)
DEFINE_EXCEPTION_STUB(174)
DEFINE_EXCEPTION_STUB(175)
DEFINE_EXCEPTION_STUB(176)
DEFINE_EXCEPTION_STUB(177)
DEFINE_EXCEPTION_STUB(178)
DEFINE_EXCEPTION_STUB(179)
DEFINE_EXCEPTION_STUB(180)
DEFINE_EXCEPTION_STUB(181)
DEFINE_EXCEPTION_STUB(182)
DEFINE_EXCEPTION_STUB(183)
DEFINE_EXCEPTION_STUB(184)
DEFINE_EXCEPTION_STUB(185)
DEFINE_EXCEPTION_STUB(186)
DEFINE_EXCEPTION_STUB(187)
DEFINE_EXCEPTION_STUB(188)
DEFINE_EXCEPTION_STUB(189)
DEFINE_EXCEPTION_STUB(190)
DEFINE_EXCEPTION_STUB(191)
DEFINE_EXCEPTION_STUB(192)
DEFINE_EXCEPTION_STUB(193)
DEFINE_EXCEPTION_STUB(194)
DEFINE_EXCEPTION_STUB(195)
DEFINE_EXCEPTION_STUB(196)
DEFINE_EXCEPTION_STUB(197)
DEFINE_EXCEPTION_STUB(198)
DEFINE_EXCEPTION_STUB(199)
DEFINE_EXCEPTION_STUB(200)
DEFINE_EXCEPTION_STUB(201)
DEFINE_EXCEPTION_STUB(202)
DEFINE_EXCEPTION_STUB(203)
DEFINE_EXCEPTION_STUB(204)
DEFINE_EXCEPTION_STUB(205)
DEFINE_EXCEPTION_STUB(206)
DEFINE_EXCEPTION_STUB(207)
DEFINE_EXCEPTION_STUB(208)
DEFINE_EXCEPTION_STUB(209)
DEFINE_EXCEPTION_STUB(210)
DEFINE_EXCEPTION_STUB(211)
DEFINE_EXCEPTION_STUB(212)
DEFINE_EXCEPTION_STUB(213)
DEFINE_EXCEPTION_STUB(214)
DEFINE_EXCEPTION_STUB(215)
DEFINE_EXCEPTION_STUB(216)
DEFINE_EXCEPTION_STUB(217)
DEFINE_EXCEPTION_STUB(218)
DEFINE_EXCEPTION_STUB(219)
DEFINE_EXCEPTION_STUB(220)
DEFINE_EXCEPTION_STUB(221)
DEFINE_EXCEPTION_STUB(222)
DEFINE_EXCEPTION_STUB(223)
DEFINE_EXCEPTION_STUB(224)
DEFINE_EXCEPTION_STUB(225)
DEFINE_EXCEPTION_STUB(226)
DEFINE_EXCEPTION_STUB(227)
DEFINE_EXCEPTION_STUB(228)
DEFINE_EXCEPTION_STUB(229)
DEFINE_EXCEPTION_STUB(230)
DEFINE_EXCEPTION_STUB(231)
DEFINE_EXCEPTION_STUB(232)
DEFINE_EXCEPTION_STUB(233)
DEFINE_EXCEPTION_STUB(234)
DEFINE_EXCEPTION_STUB(235)
DEFINE_EXCEPTION_STUB(236)
DEFINE_EXCEPTION_STUB(237)
DEFINE_EXCEPTION_STUB(238)
DEFINE_EXCEPTION_STUB(239)
DEFINE_EXCEPTION_STUB(240)
DEFINE_EXCEPTION_STUB(241)
DEFINE_EXCEPTION_STUB(242)
DEFINE_EXCEPTION_STUB(243)
DEFINE_EXCEPTION_STUB(244)
DEFINE_EXCEPTION_STUB(245)
DEFINE_EXCEPTION_STUB(246)
DEFINE_EXCEPTION_STUB(247)
DEFINE_EXCEPTION_STUB(248)
DEFINE_EXCEPTION_STUB(249)
DEFINE_EXCEPTION_STUB(250)
DEFINE_EXCEPTION_STUB(251)
DEFINE_EXCEPTION_STUB(252)
DEFINE_EXCEPTION_STUB(253)
DEFINE_EXCEPTION_STUB(254)
DEFINE_EXCEPTION_STUB(255)
