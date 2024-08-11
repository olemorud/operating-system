#pragma once

#include <stdint.h>
#include "kernel_state.h" /* kernel_memory_end */
#include "bitmap.h"

constexpr uint32_t PAGE_SIZE = 4 * 1024;

typedef uint32_t pageframe_t;

typedef uint32_t cr0_flags_t;
enum cr0_flags : cr0_flags_t {
    CR0_PROTECTED_MODE      = 1U<<0,
    CR0_MONITOR_COPROCESSOR = 1U<<1,
    CR0_EMULATION           = 1U<<2,
    CR0_TASK_SWITCHED       = 1U<<3,
    CR0_EXTENSION_TYPE      = 1U<<4,
    CR0_NUMERIC_ERROR       = 1U<<5,
    CR0_WRITE_PROTECT       = 1U<<16,
    CR0_ALIGNMENT_MASK      = 1U<<18,
    CR0_NOT_WRITETHROUGH    = 1U<<29,
    CR0_CACHE_DISABLE       = 1U<<30,
    CR0_PAGING              = 1U<<31,
};

typedef uint32_t cr4_flags_t;
enum cr4_flags : cr4_flags_t {
    CR4_VME         = 1U<<0,
    CR4_PVI         = 1U<<1,
    CR4_TSD         = 1U<<2,
    CR4_DE          = 1U<<3,
    CR4_PSE         = 1U<<4,
    CR4_PAE         = 1U<<5,
    CR4_MCE         = 1U<<6,
    CR4_PGE         = 1U<<7,
    CR4_PCE         = 1U<<8,
    CR4_OSFXSR      = 1U<<9,
    CR4_OSXMMEXCPT  = 1U<<10,
    CR4_UMIP        = 1U<<11,
    CR4_LA57        = 1U<<12,
    CR4_VMXE        = 1U<<13,
    CR4_SMXE        = 1U<<14,
    CR4_RESERVED_0  = 1U<<15,
    CR4_FSGSBASE    = 1U<<16,
    CR4_PCIDE       = 1U<<17,
    CR4_OSXSAVE     = 1U<<18,
    CR4_KL          = 1U<<19,
    CR4_SMEP        = 1U<<20,
    CR4_SMAP        = 1U<<21,
    CR4_PKE         = 1U<<22,
    CR4_CET         = 1U<<23,
    CR4_PKS         = 1U<<24,
    CR4_UINTR       = 1U<<25,
};

enum page_directory_entry_flags : pageframe_t {
    PDE_PRESENT          = 1<<0,
    PDE_WRITE            = 1<<1,
    PDE_USER_ACCESS      = 1<<2,
    PDE_WRITE_THROUGH    = 1<<3,
    PDE_DISABLE_CACHE    = 1<<4,
    PDE_ACCESSED         = 1<<5,
    PDE_DIRTY            = 1<<6,
    PDE_AVAILABLE        = 1<<6,
    PDE_4MB              = 1<<7,
    PDE_GLOBAL           = 1<<8,
    PDE_PAT              = 1<<12,
};
#define PDE_ADDRESS_4KB(x) ((x)<<12)

#define PDE_ADDRESS_4MB(x) (((x)<<13) & ~(1<<21))

enum page_table_entry_flags : pageframe_t {
    PTE_PRESENT              = 1<<0,
    PTE_WRITE                = 1<<1,
    PTE_USER                 = 1<<2,
    PTE_WRITE_THROUGH        = 1<<3,
    PTE_DISABLE_CACHE        = 1<<4,
    PTE_ACCESSED             = 1<<5,
    PTE_DIRTY                = 1<<6,
    PTE_PAGE_ATTRIBUTE_TABLE = 1<<7,
    PTE_GLOBAL               = 1<<8,
};
#define PTE_ADDRESS(x) ((x)<<12)

static inline void cr3_set(uint32_t page_directory)
{
    __asm__ volatile (
        "mov %[page_directory], %%eax\n\t"
        "mov %%eax, %%cr3\n\t"
        : /* no output */
        : /* inputs:   */ [page_directory] "m" (page_directory)
        : /* clobber:  */ "eax"
    );
}

/* these are macros so that they can compile on O0 due to the "i" constraint */
#define cr0_flags_set(flags)            \
    __asm__ volatile (                  \
        "mov %%cr0, %%eax\n\t"          \
        "or %0,     %%eax\n\t"          \
        "mov %%eax, %%cr0\n\t"          \
        : /* no outputs */              \
        : /* inputs:    */ "i"((flags)) \
        : /* clobbers:  */ "eax"        \
    )

#define cr0_flags_unset(flags)           \
    __asm__ volatile (                   \
        "mov %%cr0, %%eax\n\t"           \
        "and %0,    %%eax\n\t"           \
        "mov %%eax, %%cr0\n\t"           \
        : /* no outputs */               \
        : /* inputs:    */ "i"(~(flags)) \
        : /* clobbers:  */ "eax"         \
    )

static pageframe_t kalloc_frame(struct bitmap* frame_map, uint32_t startframe)
{
    for (size_t i = 0; i < frame_map->bit_count; i++) {
        if (bitmap_get(frame_map, i) == 0) {
            bitmap_set(frame_map, i);
            return (startframe + PTE_ADDRESS(i));
        }
    }
    return 0;
}

static pageframe_t kfree_frame(uint32_t frame)
{
    /* TODO */
    (void)frame;
    return 0;
}
