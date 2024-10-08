#pragma once

#include "types.h"

/*
 * Exceptions
 * ==========*/
__attribute__((interrupt, noreturn))
void exception_handler_div_by_zero(struct interrupt_frame* frame);

__attribute__((interrupt, noreturn))
void exception_handler_general_protection_fault(struct interrupt_frame* frame, int err);

__attribute__((interrupt, noreturn))
void exception_handler_double_fault(struct interrupt_frame* frame);

__attribute__((interrupt, noreturn))
void exception_handler_page_fault(struct interrupt_frame* frame, int err);

__attribute__((interrupt, noreturn))
void exception_default(struct interrupt_frame* frame);

/**
 * Interrupts
 * ==========
 */
__attribute__((interrupt, noreturn))
void interrupt_default(struct interrupt_frame* frame);

__attribute__((interrupt))
void interrupt_handler_1(struct interrupt_frame* frame);

/**
 * IRQs
 * ====
 */
__attribute__((interrupt)) void irq_handler_0(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_1(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_2(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_3(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_4(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_5(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_6(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_7(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_8(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_9(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_10(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_11(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_12(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_13(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_14(struct interrupt_frame*);
__attribute__((interrupt)) void irq_handler_15(struct interrupt_frame*);


/*
 * Exception and interrupt stubs
 * =============================
 * Everything below this line is a horrible compile time crime.
 * */


#include "idt.h"
#include "kernel_state.h"

/* This function is a horrible crime */
static void idt_init_stubs(struct idt_gate_descriptor idt[]) {
	#define EXCEPTION_STUB(n) exception_stub_##n

	#define DESC(func) \
		idt_encode_descriptor( \
				func, \
				segment(SEGMENT_KERNEL_CODE, SEGMENT_GDT, 0), \
				IDT_DPL_3, \
				IDT_GATE_TYPE_TRAP32)

	#define SET_EXCEPTION_STUB(n) \
		__attribute__((interrupt)) void EXCEPTION_STUB(n)(struct interrupt_frame*, uint32_t err); \
		idt[n] = DESC(EXCEPTION_STUB(n))
	SET_EXCEPTION_STUB(0);
	SET_EXCEPTION_STUB(1);
	SET_EXCEPTION_STUB(2);
	SET_EXCEPTION_STUB(3);
	SET_EXCEPTION_STUB(4);
	SET_EXCEPTION_STUB(5);
	SET_EXCEPTION_STUB(6);
	SET_EXCEPTION_STUB(7);
	SET_EXCEPTION_STUB(8);
	SET_EXCEPTION_STUB(9);
	SET_EXCEPTION_STUB(10);
	SET_EXCEPTION_STUB(11);
	SET_EXCEPTION_STUB(12);
	SET_EXCEPTION_STUB(13);
	SET_EXCEPTION_STUB(14);
	SET_EXCEPTION_STUB(15);
	SET_EXCEPTION_STUB(16);
	SET_EXCEPTION_STUB(17);
	SET_EXCEPTION_STUB(18);
	SET_EXCEPTION_STUB(19);
	SET_EXCEPTION_STUB(20);
	SET_EXCEPTION_STUB(21);
	SET_EXCEPTION_STUB(22);
	SET_EXCEPTION_STUB(23);
	SET_EXCEPTION_STUB(24);
	SET_EXCEPTION_STUB(25);
	SET_EXCEPTION_STUB(26);
	SET_EXCEPTION_STUB(27);
	SET_EXCEPTION_STUB(28);
	SET_EXCEPTION_STUB(29);
	SET_EXCEPTION_STUB(30);
	SET_EXCEPTION_STUB(31);
	SET_EXCEPTION_STUB(32);
	SET_EXCEPTION_STUB(33);
	SET_EXCEPTION_STUB(34);
	SET_EXCEPTION_STUB(35);
	SET_EXCEPTION_STUB(36);
	SET_EXCEPTION_STUB(37);
	SET_EXCEPTION_STUB(38);
	SET_EXCEPTION_STUB(39);
	SET_EXCEPTION_STUB(40);
	SET_EXCEPTION_STUB(41);
	SET_EXCEPTION_STUB(42);
	SET_EXCEPTION_STUB(43);
	SET_EXCEPTION_STUB(44);
	SET_EXCEPTION_STUB(45);
	SET_EXCEPTION_STUB(46);
	SET_EXCEPTION_STUB(47);
	SET_EXCEPTION_STUB(48);
	SET_EXCEPTION_STUB(49);
	SET_EXCEPTION_STUB(50);
	SET_EXCEPTION_STUB(51);
	SET_EXCEPTION_STUB(52);
	SET_EXCEPTION_STUB(53);
	SET_EXCEPTION_STUB(54);
	SET_EXCEPTION_STUB(55);
	SET_EXCEPTION_STUB(56);
	SET_EXCEPTION_STUB(57);
	SET_EXCEPTION_STUB(58);
	SET_EXCEPTION_STUB(59);
	SET_EXCEPTION_STUB(60);
	SET_EXCEPTION_STUB(61);
	SET_EXCEPTION_STUB(62);
	SET_EXCEPTION_STUB(63);
	SET_EXCEPTION_STUB(64);
	SET_EXCEPTION_STUB(65);
	SET_EXCEPTION_STUB(66);
	SET_EXCEPTION_STUB(67);
	SET_EXCEPTION_STUB(68);
	SET_EXCEPTION_STUB(69);
	SET_EXCEPTION_STUB(70);
	SET_EXCEPTION_STUB(71);
	SET_EXCEPTION_STUB(72);
	SET_EXCEPTION_STUB(73);
	SET_EXCEPTION_STUB(74);
	SET_EXCEPTION_STUB(75);
	SET_EXCEPTION_STUB(76);
	SET_EXCEPTION_STUB(77);
	SET_EXCEPTION_STUB(78);
	SET_EXCEPTION_STUB(79);
	SET_EXCEPTION_STUB(80);
	SET_EXCEPTION_STUB(81);
	SET_EXCEPTION_STUB(82);
	SET_EXCEPTION_STUB(83);
	SET_EXCEPTION_STUB(84);
	SET_EXCEPTION_STUB(85);
	SET_EXCEPTION_STUB(86);
	SET_EXCEPTION_STUB(87);
	SET_EXCEPTION_STUB(88);
	SET_EXCEPTION_STUB(89);
	SET_EXCEPTION_STUB(90);
	SET_EXCEPTION_STUB(91);
	SET_EXCEPTION_STUB(92);
	SET_EXCEPTION_STUB(93);
	SET_EXCEPTION_STUB(94);
	SET_EXCEPTION_STUB(95);
	SET_EXCEPTION_STUB(96);
	SET_EXCEPTION_STUB(97);
	SET_EXCEPTION_STUB(98);
	SET_EXCEPTION_STUB(99);
	SET_EXCEPTION_STUB(100);
	SET_EXCEPTION_STUB(101);
	SET_EXCEPTION_STUB(102);
	SET_EXCEPTION_STUB(103);
	SET_EXCEPTION_STUB(104);
	SET_EXCEPTION_STUB(105);
	SET_EXCEPTION_STUB(106);
	SET_EXCEPTION_STUB(107);
	SET_EXCEPTION_STUB(108);
	SET_EXCEPTION_STUB(109);
	SET_EXCEPTION_STUB(110);
	SET_EXCEPTION_STUB(111);
	SET_EXCEPTION_STUB(112);
	SET_EXCEPTION_STUB(113);
	SET_EXCEPTION_STUB(114);
	SET_EXCEPTION_STUB(115);
	SET_EXCEPTION_STUB(116);
	SET_EXCEPTION_STUB(117);
	SET_EXCEPTION_STUB(118);
	SET_EXCEPTION_STUB(119);
	SET_EXCEPTION_STUB(120);
	SET_EXCEPTION_STUB(121);
	SET_EXCEPTION_STUB(122);
	SET_EXCEPTION_STUB(123);
	SET_EXCEPTION_STUB(124);
	SET_EXCEPTION_STUB(125);
	SET_EXCEPTION_STUB(126);
	SET_EXCEPTION_STUB(127);
	SET_EXCEPTION_STUB(128);
	SET_EXCEPTION_STUB(129);
	SET_EXCEPTION_STUB(130);
	SET_EXCEPTION_STUB(131);
	SET_EXCEPTION_STUB(132);
	SET_EXCEPTION_STUB(133);
	SET_EXCEPTION_STUB(134);
	SET_EXCEPTION_STUB(135);
	SET_EXCEPTION_STUB(136);
	SET_EXCEPTION_STUB(137);
	SET_EXCEPTION_STUB(138);
	SET_EXCEPTION_STUB(139);
	SET_EXCEPTION_STUB(140);
	SET_EXCEPTION_STUB(141);
	SET_EXCEPTION_STUB(142);
	SET_EXCEPTION_STUB(143);
	SET_EXCEPTION_STUB(144);
	SET_EXCEPTION_STUB(145);
	SET_EXCEPTION_STUB(146);
	SET_EXCEPTION_STUB(147);
	SET_EXCEPTION_STUB(148);
	SET_EXCEPTION_STUB(149);
	SET_EXCEPTION_STUB(150);
	SET_EXCEPTION_STUB(151);
	SET_EXCEPTION_STUB(152);
	SET_EXCEPTION_STUB(153);
	SET_EXCEPTION_STUB(154);
	SET_EXCEPTION_STUB(155);
	SET_EXCEPTION_STUB(156);
	SET_EXCEPTION_STUB(157);
	SET_EXCEPTION_STUB(158);
	SET_EXCEPTION_STUB(159);
	SET_EXCEPTION_STUB(160);
	SET_EXCEPTION_STUB(161);
	SET_EXCEPTION_STUB(162);
	SET_EXCEPTION_STUB(163);
	SET_EXCEPTION_STUB(164);
	SET_EXCEPTION_STUB(165);
	SET_EXCEPTION_STUB(166);
	SET_EXCEPTION_STUB(167);
	SET_EXCEPTION_STUB(168);
	SET_EXCEPTION_STUB(169);
	SET_EXCEPTION_STUB(170);
	SET_EXCEPTION_STUB(171);
	SET_EXCEPTION_STUB(172);
	SET_EXCEPTION_STUB(173);
	SET_EXCEPTION_STUB(174);
	SET_EXCEPTION_STUB(175);
	SET_EXCEPTION_STUB(176);
	SET_EXCEPTION_STUB(177);
	SET_EXCEPTION_STUB(178);
	SET_EXCEPTION_STUB(179);
	SET_EXCEPTION_STUB(180);
	SET_EXCEPTION_STUB(181);
	SET_EXCEPTION_STUB(182);
	SET_EXCEPTION_STUB(183);
	SET_EXCEPTION_STUB(184);
	SET_EXCEPTION_STUB(185);
	SET_EXCEPTION_STUB(186);
	SET_EXCEPTION_STUB(187);
	SET_EXCEPTION_STUB(188);
	SET_EXCEPTION_STUB(189);
	SET_EXCEPTION_STUB(190);
	SET_EXCEPTION_STUB(191);
	SET_EXCEPTION_STUB(192);
	SET_EXCEPTION_STUB(193);
	SET_EXCEPTION_STUB(194);
	SET_EXCEPTION_STUB(195);
	SET_EXCEPTION_STUB(196);
	SET_EXCEPTION_STUB(197);
	SET_EXCEPTION_STUB(198);
	SET_EXCEPTION_STUB(199);
	SET_EXCEPTION_STUB(200);
	SET_EXCEPTION_STUB(201);
	SET_EXCEPTION_STUB(202);
	SET_EXCEPTION_STUB(203);
	SET_EXCEPTION_STUB(204);
	SET_EXCEPTION_STUB(205);
	SET_EXCEPTION_STUB(206);
	SET_EXCEPTION_STUB(207);
	SET_EXCEPTION_STUB(208);
	SET_EXCEPTION_STUB(209);
	SET_EXCEPTION_STUB(210);
	SET_EXCEPTION_STUB(211);
	SET_EXCEPTION_STUB(212);
	SET_EXCEPTION_STUB(213);
	SET_EXCEPTION_STUB(214);
	SET_EXCEPTION_STUB(215);
	SET_EXCEPTION_STUB(216);
	SET_EXCEPTION_STUB(217);
	SET_EXCEPTION_STUB(218);
	SET_EXCEPTION_STUB(219);
	SET_EXCEPTION_STUB(220);
	SET_EXCEPTION_STUB(221);
	SET_EXCEPTION_STUB(222);
	SET_EXCEPTION_STUB(223);
	SET_EXCEPTION_STUB(224);
	SET_EXCEPTION_STUB(225);
	SET_EXCEPTION_STUB(226);
	SET_EXCEPTION_STUB(227);
	SET_EXCEPTION_STUB(228);
	SET_EXCEPTION_STUB(229);
	SET_EXCEPTION_STUB(230);
	SET_EXCEPTION_STUB(231);
	SET_EXCEPTION_STUB(232);
	SET_EXCEPTION_STUB(233);
	SET_EXCEPTION_STUB(234);
	SET_EXCEPTION_STUB(235);
	SET_EXCEPTION_STUB(236);
	SET_EXCEPTION_STUB(237);
	SET_EXCEPTION_STUB(238);
	SET_EXCEPTION_STUB(239);
	SET_EXCEPTION_STUB(240);
	SET_EXCEPTION_STUB(241);
	SET_EXCEPTION_STUB(242);
	SET_EXCEPTION_STUB(243);
	SET_EXCEPTION_STUB(244);
	SET_EXCEPTION_STUB(245);
	SET_EXCEPTION_STUB(246);
	SET_EXCEPTION_STUB(247);
	SET_EXCEPTION_STUB(248);
	SET_EXCEPTION_STUB(249);
	SET_EXCEPTION_STUB(250);
	SET_EXCEPTION_STUB(251);
	SET_EXCEPTION_STUB(252);
	SET_EXCEPTION_STUB(253);
	SET_EXCEPTION_STUB(254);
	SET_EXCEPTION_STUB(255);
	#undef DESC
	#undef SET_EXCEPTION_STUB
    #undef EXCEPTION_STUB
}
