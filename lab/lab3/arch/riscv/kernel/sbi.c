#include "defs.h" 
#include "riscv.h"

uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret_val;
    __asm__ volatile ( 
        "mv x17, %[type]\n"
        "mv x10,%[src0]\n"
        "mv x11,%[src1]\n"
        "mv x12,%[src2]\n"
        "ecall\n"
        "mv %[dest],x10"
        : [dest] "=r" (ret_val) 
        : [type] "r" (sbi_type), [src0] "r" (arg0), [src1] "r" (arg1), [src2] "r" (arg2)
        : "memory"
   );
    return ret_val;
}


void trigger_time_interrupt(unsigned long long stime_value) {
    sbi_call(0, stime_value, 0, 0);
}
