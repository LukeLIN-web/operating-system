.section .text.entry

.extern test
.global trap_s
.extern handler_s
.equ reg_size, 0x8
.align 2

trap_s:
	# save caller-saved registers and spec
	sd sp, -8(sp)  
    sd ra, -16(sp)  
    sd gp, -24(sp)  
    sd tp, -32(sp)  
    sd t0, -40(sp)  
    sd t1, -48(sp)  
    sd t2, -56(sp)
    sd s0, -64(sp)  
    sd s1, -72(sp)  
    sd a0, -80(sp)  
    sd a1, -88(sp)  
    sd a2, -96(sp)  
    sd a3, -104(sp)  
    sd a4, -112(sp)  
    sd a5, -120(sp)  
    sd a6, -128(sp)  
    sd a7, -136(sp)  
    sd s2, -144(sp)  
    sd s3, -152(sp)  
    sd s4, -160(sp)  
    sd s5, -168(sp)  
    sd s6, -176(sp)  
    sd s7, -184(sp)  
    sd s8, -192(sp)  
    sd s9, -200(sp)  
    sd s10, -208(sp) 
    sd s11, -216(sp) 
    sd t3, -224(sp)  
    sd t4, -232(sp)  
    csrrs t5,sepc,zero
    sd t5, -240(sp)  
    sd t6, -248(sp)  
    addi sp, sp, -248
    csrrs a0,scause,zero
	# call handler_s(scause)
	call handler_s
	# load sepc and caller-saved registers
	ld t6, 0(sp)  
    ld t5, 8(sp)  
    csrw sepc, t5
    ld t4, 16(sp)  
    ld t3, 24(sp)  
    ld s11, 32(sp)  
    ld s10, 40(sp)  
    ld s9, 48(sp)  
    ld s8, 56(sp)  
    ld s7, 64(sp)  
    ld s6, 72(sp)  
    ld s5, 80(sp)  
    ld s4, 88(sp)  
    ld s3, 96(sp)  
    ld s2, 104(sp)  
    ld a7, 112(sp)  
    ld a6, 120(sp)  
    ld a5, 128(sp)  
    ld a4, 136(sp)  
    ld a3, 144(sp)  
    ld a2, 152(sp)  
    ld a1, 160(sp)  
    ld a0, 168(sp)  
    ld s1, 176(sp)  
    ld s0, 184(sp)  
    ld t2, 192(sp)  
    ld t1, 200(sp)  
    ld t0, 208(sp)
    ld tp, 216(sp)  
    ld gp, 224(sp)  
    ld ra, 232(sp)  
    ld sp, 240(sp)  
	sret


.globl __switch_to
__switch_to:
	li    a4,  40 
	add   a3, a0, a4
	add   a4, a1, a4
	# Save context into prev->thread
	sd ra,0(a3)
	sd sp,8(a3)
	sd s0,16(a3)
	sd s1,24(a3)
	sd s2,32(a3)
	sd s3,40(a3)
	sd s4,48(a3)
	sd s5,56(a3)
	sd s6,64(a3)
	sd s7,72(a3)
	sd s8,80(a3)
	sd s9,88(a3)
	sd s10,96(a3)
	sd s11,104(a3)
	# Restore context from next->thread
	ld ra,0(a4)
	ld sp,8(a4)
	ld s0,16(a4)
	ld s1,24(a4)
	ld s2,32(a4)
	ld s3,40(a4)
	ld s4,48(a4)
	ld s5,56(a4)
	ld s6,64(a4)
	ld s7,72(a4)
	ld s8,80(a4)
	ld s9,88(a4)
	ld s10,96(a4)
	ld s11,104(a4)
	# return to ra
	ret


.globl __init_sepc
__init_sepc:
	la  t0, test
	csrw sepc, t0
	sret
