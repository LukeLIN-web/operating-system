#include "put.h"
#include "sched.h"
#include "syscall.h"
int cntTimeI;
int cntPut;

void handler_s(size_t scause, size_t sepc, uintptr_t *regs) {
    if (scause == 12 || scause == 13 || scause == 15) {
        puts("page fault\n");
        while(1) ;
    } else if (scause == (((unsigned long)1 << 63) | 5)){
        do_timer();
    } else if (scause == ((unsigned long)8)) {
        __asm__ __volatile__("csrr t0, sepc\n\taddi t0, t0, 4\n\tcsrrw x0, sepc, t0\n\t");
        unsigned long a7 = regs[16];
        if (a7 == 64) {
            unsigned long a0 = sys_write(regs[9], regs[10], regs[11]);
            regs[9] = a0;
        } else if (a7 == 172) {
            unsigned long a0 = sys_getpid();
            regs[9] = a0;
        }
    }
}