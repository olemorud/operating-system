
#include "syscall.h"

#include "libc.h"
#include "tty.h"
#include "interrupts.h"
#include "kernel_state.h"
#include "file_system.h"

#define EXCEPTION_DEPTH_MAX 3

int syscall_main(int syscall_no, int a, int b, int c)
{
    int status = 0;

    switch (syscall_no) {

    case SYSCALL_PRINT:
    {
        terminal_write(*(struct str*)a);
    } break;

    case SYSCALL_OPEN:
    {
        struct str path = *(struct str*)a;
        struct file_path* f = file_open(&g_ramdisk, path);
    } break;

    case SYSCALL_WRITE:
    {
        //bool file_write(struct file_system* fs, struct file_path* f, const uint8_t* data, size_t n);
        struct file_path* filepath = (struct file_path*)a;
        const uint8_t* data        = (const uint8_t*)b;
        const uint8_t n            = (const uint8_t)c;
        file_write(&g_ramdisk, filepath, data, n);
    } break;

    case SYSCALL_READ:
    {
        //bool file_read(struct file_system* fs, struct file_path* f, uint8_t* out, int n);
        struct file_path* filepath = (struct file_path*)a;
        uint8_t* out               = (uint8_t*)b;
        const uint8_t n            = (const uint8_t)c;
        file_read(&g_ramdisk, filepath, out, n);
    } break;

    case SYSCALL_EXIT:
    {
    
    } break;

    default:
    {
        printf(str_attach("unknown syscall {i32}: a:{i32}, b:{i32}, c:{i32}\n"), syscall_no, a, b, c);
    } break;
    
    }

    return 0;
}
