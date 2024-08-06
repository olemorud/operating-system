#pragma once
#include <stdint.h>

struct __attribute__((packed)) tss {
    /* Contains the Segment Selector for the TSS of the previous task, with
       hardware task switching these form a kind of backward linked list. */
    uint16_t link;
    uint16_t link_reserved;
    
    /* the stack pointer to load when changing to kernel mode */
    uint32_t esp0;

    /* the stack segment pointer to load when changing to kernel mode */
    uint16_t ss0;
                 
    uint16_t ss0_reserved;

    /* everything under here is unused in software task switching */
    struct __attribute__((packed)) {
        /* esp and ss 1 and 2 would be used when switching to ring 1 and 2 */
        uint32_t esp1;
        uint16_t ss1;
        uint16_t ss1_reserved;
        uint32_t esp2;
        uint16_t ss2;
        uint16_t ss2_reserved;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint16_t es; 
        uint16_t es_reserved; 
        uint16_t cs; 
        uint16_t cs_reserved; 
        uint16_t ss; 
        uint16_t ss_reserved; 
        uint16_t ds; 
        uint16_t ds_reserved; 
        uint16_t fs; 
        uint16_t fs_reserved; 
        uint16_t gs; 
        uint16_t gs_reserved; 
        uint16_t ldtr; 
        uint16_t ldtr_reserved; 
        uint16_t reserved;
        uint16_t iopb;
        uint32_t ssp;
    } unused;
};

/* offset: the offset in bytes of the tss descriptor in the gdt table */
static inline void tss_load(uint32_t offset)
{
    __asm__ volatile (
            "mov %[offset], %%ax\n\t"
            "ltr %%ax"
            :
            : [offset] "m"(offset)
            : "ax");
}

