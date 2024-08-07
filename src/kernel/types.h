
#pragma once


typedef void(*func_t)(void*);

/* define uword_t per the instruction set manual */
#ifdef __x86_64__
typedef unsigned long long int uword_t;
#else
typedef unsigned int uword_t;
#endif

/* segment selectors identifies segments either in the gdt or the ldt */
typedef uint16_t segment_t;

enum segment_ti : uint16_t {
	SEGMENT_GDT = 0,
	SEGMENT_LDT = 1,
};

static inline segment_t segment(uint16_t index, enum segment_ti ti, uint16_t rpl)
{
	return (index << 3) | ((ti & 0xF) << 1) | (rpl & 0xFF);
}


