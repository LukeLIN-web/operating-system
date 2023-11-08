.section .text.entry
.global trap_s
.global skip_strap
.global s_ret
.global entry_s
.global switch_context
.global task_first_ret
.global ret_from_fork

entry_s:
    jal x1, init_buddy_system
    jal x1, paging_init
    addi t0, x0, 1
    slli t0, t0, 63
    add t1, x0, a0
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
    bne sp, x0, switch_from_user
    csrr sp, sscratch
switch_from_user:
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
    csrr t0, sepc
    sd t0, 248(sp)
    csrr t0, sstatus
    sd t0, 256(sp)
    csrr t0, stval
    sd t0, 264(sp)
    csrr t0, scause
    sd t0, 272(sp)

    la a0, current
    ld a0, 0(a0)
    ld a1, 8(sp)
    sd a1, 160(a0)
    csrr a1, sscratch
    sd a1, 8(sp)
    csrw sscratch, zero

    # csrrw sp, sscratch, sp

    ld a0, 272(sp)
    ld a1, 248(sp)
    add a2, sp, x0

    jal x1, handler_s

    skip_strap:
    # j show_page_tables
    
    ecall

    # csrrw sp, sscratch, sp

s_ret:
    ld a1, 8(sp)
    csrw sscratch, a1
    la a0, current
    ld a0, 0(a0)
    ld a1, 160(a0)
    sd a1, 8(sp)

    ld t0, 248(sp)
    csrw sepc, t0
    ld t0, 256(sp)
    csrw sstatus, t0
    ld t0, 264(sp)
    csrw stval, t0
    ld t0, 272(sp)
    csrw scause, t0

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
    ld x1, 0(sp)
    ld x2, 8(sp)
    addi sp, sp, 280
    debug_for_sp:
    csrrw sp, sscratch, sp
    sret


switch_context:
    sd ra, 0(a0)
    sd sp, 8(a0)
    sd s0, 16(a0)
    sd s1, 24(a0)
    sd s2, 32(a0)
    sd s3, 40(a0)
    sd s4, 48(a0)
    sd s5, 56(a0)
    sd s6, 64(a0)
    sd s7, 72(a0)
    sd s8, 80(a0)
    sd s9, 88(a0)
    sd s10, 96(a0)
    sd s11, 104(a0)
    ld ra, 0(a1)
    ld sp, 8(a1)
    ld s0, 16(a1)
    ld s1, 24(a1)
    ld s2, 32(a1)
    ld s3, 40(a1)
    ld s4, 48(a1)
    ld s5, 56(a1)
    ld s6, 64(a1)
    ld s7, 72(a1)
    ld s8, 80(a1)
    ld s9, 88(a1)
    ld s10, 96(a1)
    ld s11, 104(a1)
    jr ra


task_first_ret:
    la t0, current
    ld t0, 0(t0)
    ld t0, 160(t0)
    csrw sscratch, t0
    la t0, current
    ld t0, 0(t0)
    ld t0, 152(t0)
    csrw sepc, t0
    ecall
    li t0, 256
    csrc sstatus, t0
    li t0, 262144
    csrs sstatus, t0
    sret

ret_from_fork:
    ld t0, 248(a0)
    csrw sepc, t0
    ld t0, 256(a0)
    csrw sstatus, t0
    ld t0, 264(a0)
    csrw stval, t0
    ld t0, 272(a0)
    csrw scause, t0
    ld x31, 240(a0)
    ld x30, 232(a0)
    ld x29, 224(a0)
    ld x28, 216(a0)
    ld x27, 208(a0)
    ld x26, 200(a0)
    ld x25, 192(a0)
    ld x24, 184(a0)
    ld x23, 176(a0)
    ld x22, 168(a0)
    ld x21, 160(a0)
    ld x20, 152(a0)
    ld x19, 144(a0)
    ld x18, 136(a0)
    ld x17, 128(a0)
    ld x16, 120(a0)
    ld x15, 112(a0)
    ld x14, 104(a0)
    ld x13, 96(a0)
    ld x12, 88(a0)
    ld x11, 80(a0)
    ld x9, 64(a0)
    ld x8, 56(a0)
    ld x7, 48(a0)
    ld x6, 40(a0)
    ld x5, 32(a0)
    ld x4, 24(a0)
    ld x3, 16(a0)
    ld x2, 8(a0)
    ld x1, 0(a0)
    ld x10, 72(a0)
    sret