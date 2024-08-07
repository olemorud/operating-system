
#include "str.h"

#include "kernel_state.h"

const struct str gdt_segment_index_str[SEGMENT_COUNT] = {
    [SEGMENT_NULL       ] = str_attach("SEGMENT_NULL"),
    [SEGMENT_KERNEL_CODE] = str_attach("SEGMENT_KERNEL_CODE"),
    [SEGMENT_KERNEL_DATA] = str_attach("SEGMENT_KERNEL_DATA"),
    [SEGMENT_USER_CODE  ] = str_attach("SEGMENT_USER_CODE"),
    [SEGMENT_USER_DATA  ] = str_attach("SEGMENT_USER_DATA"),
    [SEGMENT_TASK_STATE ] = str_attach("SEGMENT_TASK_STATE"),
};

const struct str idt_index_str[IDT_COUNT] = {
};

struct kernel_state kernel = { 0 };
