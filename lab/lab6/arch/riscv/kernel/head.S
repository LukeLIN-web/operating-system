.section .text.init
.global _start
_start:
    csrrs x22, mstatus, x0
    csrrw x0, satp, x0
    andi x22, x22, -256
    csrrw x0, mstatus, x22
    lui t3, 2
    addi t3, t3, -2048
    csrrc x0, mstatus, t3
    lui t3, 1
    addi t3, t3, -2048
    csrrs x0, mstatus, t3
    ori t3, x0, 1
    slli t3, t3, 18
    csrrs x0, mstatus, t3


    la t3, trap_m
    csrrw x0, mtvec, t3
    la t3, trap_s
    csrrw x0, stvec, t3

    la sp, stack_top
    csrrw x0, mscratch, sp


    la t5, .bss
    la t6, bss_end
    beq t5, t6, skip_bss_clear_loop
    bss_clear_loop:
    sb x0, 0(t5)
    addi t5, t5, 1
    bne t5, t6, bss_clear_loop
    skip_bss_clear_loop:

    add t3, x0, x0
    li t3, 33603576

    ld x24, 0(t3)

    add t4, x0, x0
    lui t4, 244
    ori t4, t4, 576
    add t4, t4, x24

    add t5, x0, x0
    lui t5, 8196
    sd t4, 0(t5)

    csrrw x0, mideleg, x0
    ori t3, x0, 32
    csrrs x0, mideleg, t3

    ori t3, x0, 1
    slli t3, t3, 12
    csrrs x0, medeleg, t3
    ori t3, x0, 1
    slli t3, t3, 13
    csrrs x0, medeleg, t3
    ori t3, x0, 1
    slli t3, t3, 15
    csrrs x0, medeleg, t3
    ori t3, x0, 1
    slli t3, t3, 8
    csrrs x0, medeleg, t3



    ori t3, x0, 128
    ori t3, t3, 8
    csrrs x0, mstatus, t3

    ori t3, x0, 2
    csrrs x0, sstatus, t3

    ori t3, x0, 128
    csrrs x0, mie, t3

    la t4, entry_s
    csrrw  x0, mepc, t4
    mret


trap_m:
    csrrw sp, mscratch, sp
    addi sp, sp, -280
    sd x1, 0(sp)
    # sd x2, 8(sp)
    sd x3, 16(sp)
    sd x4, 24(sp)
    sd x5, 32(sp)
    sd x6, 40(sp)
    sd x7, 48(sp)
    sd x8, 56(sp)
    sd x9, 64(sp)
    sd x10, 72(sp)
    sd x11, 80(sp)
    sd x12, 88(sp)
    sd x13, 96(sp)
    sd x14, 104(sp)
    sd x15, 112(sp)
    sd x16, 120(sp)
    sd x17, 128(sp)
    sd x18, 136(sp)
    sd x19, 144(sp)
    sd x20, 152(sp)
    sd x21, 160(sp)
    sd x22, 168(sp)
    sd x23, 176(sp)
    sd x24, 184(sp)
    sd x25, 192(sp)
    sd x26, 200(sp)
    sd x27, 208(sp)
    sd x28, 216(sp)
    sd x29, 224(sp)
    sd x30, 232(sp)
    sd x31, 240(sp)
    csrrs t0, mepc, x0
    sd t0, 248(sp)
    csrrs t0, mcause, x0
    sd t0, 256(sp)
    csrrs t0, mstatus, x0
    sd t0, 264(sp)
    csrrs t0, mtval, x0
    sd t0, 272(sp)

    addi t0, x0, 1
    sll t0, t0, 63
    addi t0, t0, 7
    csrrs t2, mcause, x0
    bne t2, t0, skip_time_interrupt
    jal t1, time_interrupt
    skip_time_interrupt:
    addi t0, x0, 9
    csrrs t2, mcause, x0
    bne t2, t0, skip_s_ecall
    jal t1, s_ecall
    skip_s_ecall:

    ld t0, 272(sp)
    csrrw x0, mtval, t0
    ld t0, 264(sp)
    csrrw x0, mstatus, t0
    ld t0, 256(sp)
    csrrw x0, mcause, t0
    ld t0, 248(sp)
    csrrw x0, mepc, t0
    ld x31, 240(sp)
    ld x30, 232(sp)
    ld x29, 224(sp)
    ld x28, 216(sp)
    ld x27, 208(sp)
    ld x26, 200(sp)
    ld x25, 192(sp)
    ld x24, 184(sp)
    ld x23, 176(sp)
    ld x22, 168(sp)
    ld x21, 160(sp)
    ld x20, 152(sp)
    ld x19, 144(sp)
    ld x18, 136(sp)
    ld x17, 128(sp)
    ld x16, 120(sp)
    ld x15, 112(sp)
    ld x14, 104(sp)
    ld x13, 96(sp)
    ld x12, 88(sp)
    ld x11, 80(sp)
    ld x10, 72(sp)
    ld x9, 64(sp)
    ld x8, 56(sp)
    ld x7, 48(sp)
    ld x6, 40(sp)
    ld x5, 32(sp)
    ld x4, 24(sp)
    ld x3, 16(sp)
    # ld x2, 8(sp)
    ld x1, 0(sp)
    addi sp, sp, 280
    csrrw sp, mscratch, sp
    mret

time_interrupt:

    ori t3, x0, 128
    csrrc x0, mie, t3
    ori t3, x0, 32
    csrrw x0, mip, t3
    jalr x0, t1, 0

s_ecall:
    ld t3, 248(sp)
    addi t3, t3, 4
    sd t3, 248(sp)

    add t5, x0, x0
    lui t5, 8196
    ld t2, 0(t5)

    add t4, x0, x0
    li t4, 1000000
    add t2, t2, t4
    
    sd t2, 0(t5)

    ori t3, x0, 128
    csrrs x0, mie, t3
    csrrw x0, mip, x0
ecall_jal:
    jalr x0, t1, 0

