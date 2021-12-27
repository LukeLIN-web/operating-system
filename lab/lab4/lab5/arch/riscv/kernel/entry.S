.section .text.entry
.global trap_s
.global skip_strap
.global s_ret
.global entry_s

entry_s:
    jal x1, paging_init
    addi t0, x0, 1
    slli t0, t0, 63
    la t1, _end
    srli t1, t1, 12
    or t0, t0, t1
    csrrw x0, satp, t0
    addi t0, x0, 1
    slli t0, t0, 27
    addi t0, t0, -1
    slli t0, t0, 37

    addi t1, x0, 1
    slli t1, t1, 31
    sub t0, t0, t1

    la t1, trap_s
    add t1, t1, t0
    csrrw x0, stvec, t1
    la t1, init_stack_top
    add sp, t1, t0
    
    ori t3, x0, 32
    csrrs x0, sie, t3

    la t1, start_kernel
    add t1, t1, t0
    jalr x1, t1, 0

trap_s:
    csrrw sp, sscratch, sp

    addi sp, sp, -280

    sd x1, 0(sp)
    sd x2, 8(sp)
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
    # csrrw sp, sscratch, sp

    csrr a0, scause
    csrr a1, sepc
    add a2, sp, x0

    jal x1, handler_s

    skip_strap:
    ecall
    # csrrw sp, sscratch, sp

s_ret:
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
    ld x2, 8(sp)
    ld x1, 0(sp)
    addi sp, sp, 280
    debug_for_sp:
    csrrw sp, sscratch, sp
    sret
