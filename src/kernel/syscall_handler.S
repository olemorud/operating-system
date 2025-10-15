.global interrupt_handler_syscall
.align 4

interrupt_handler_syscall:  

pushal // push all general purpose registers
cld    // clear direction flag

// Passing Parameters
// (https://gitlab.com/x86-psABIs/i386-ABI/-/wikis/uploads/14c05f1b1e156e0e46b61bfa7c1df1e2/intel386-psABI-2020-08-07.pdf)
// ---------
// Most parameters are passed on the stack. Parameters are pushed onto the
// stack in reverse order - the last argument in the parameter list has the
// highest address, that is, it is stored farthest away from the stack pointer
// at the time of the call.


push %edx 
push %ecx 
push %ebx 
push %eax 
call syscall_main
pop %eax 
pop %ebx 
pop %ecx 
pop %edx 

popal // pop all general purpose registers

iret
