#include "put.h"
#include "sched.h"
#include "syscall.h"
#include "fault.h"
int cntTimeI;
int cntPut;

void handler_s(size_t scause, size_t sepc, uintptr_t *regs) {
    current->thread.sepc = sepc;
    current->thread.stack = regs;
    if (scause == 12 || scause == 13 || scause == 15) {
        do_page_fault();
    } else if (scause == (((unsigned long)1 << 63) | 5)){
        do_timer();
    } else if (scause == ((unsigned long)8)) {
        regs[31] += 4;
        unsigned long a7 = regs[16];
        if (a7 == 64) {
            unsigned long a0 = sys_write(regs[9], regs[10], regs[11]);
            regs[9] = a0;
        } else if (a7 == 172) {
            unsigned long a0 = sys_getpid();
            regs[9] = a0;
        } else if (a7 == 222) {
            unsigned long a0 = mmap(regs[9], regs[10], regs[11], regs[12], regs[13], regs[14]);
            regs[9] = a0;
        } else if (a7 == 215) {
            unsigned long a0 = munmap(regs[9], regs[10]);
            regs[9] = a0;
        } else if (a7 == 226) {
            unsigned long a0 = mprotect(regs[9], regs[10], regs[11]);
            regs[9] = a0;
        } else if (a7 == 220) {
            unsigned long a0 = fork();
            regs[9] = a0;
        }
    }
}