.align 3
.section .text.init
.globl _start
.globl _supervisor
.globl _mtrap
.globl clean_loop
.globl time_interupt
.globl encall_from_s
.globl exit
.globl is_int
.globl other_trap
.extern start_kernel
.extern stack_top
.extern trap_s 
.extern bss_start
.extern bss_end
.extern _end
.extern paging_init
.extern init_stack_top

_start:
	# 关闭所有中断
	li t1, 0x8               # t1=1000
	csrc mstatus, t1         # 将mstatus寄存器的第三位置0（1的对应位清零）
	li t1, 0x888             # t1=1000 1000 1000
	csrc mie, t1             # 将mie寄存器的第11、7、3位置0（1的对应位清零）

	# 设置M异常地址
	la t1, _mtrap            # t1=&_mtrap
	csrw mtvec, t1           

	# 设置mscratch 保存M mode下上下文物理空间的地址
	la t1, stack_top
	csrw mscratch, t1

	# 设置
	li t1, 0xb000
    csrs medeleg,t1

	# 初始化.bss
	la t1, bss_start         # t1=bss段start的地址
	la t2, bss_end           # t2=bss段end的地址
	li t3, 0x0               # 填入的内容

clean_loop:
	sb t3, 0(t1)             #往t1的内存中写入t3的内容（每次1个字节）
	addi t1, t1, 1           #t1的值往后1个字节
	bne t1, t2, clean_loop   #比较，如果t1和t2的值不等，则继续往内存中存t3的内容

	# 初始化mtimecmp寄存器
	li t1, 0x200bff8         #t1=mtime的地址
	li t2, 0x2004000         #t2=mtimecmp的地址
	ld t4, 0(t1)             #读mtime的8字节的内容到t4中
	li t3, 1000000         # 控制时间  原来的值是 1000 000
	add t4, t4, t3           # t4加上1000000
	sd t4, 0(t2)             # 将a0的值存入地址为t2的内存中（8字节）

	# 设置时钟中断委托
	li t1, 0x20              # 10 0000     mideleg的第5位置1
	csrs mideleg, t1

	# 设置instruction/load/store page fault委托
	li t1, 0xB
	slli t1, t1, 12				#t1=1011 0000 0000 0000
	csrs medeleg, t1

	li t1, 0x12a             #0001 0010 1010   初始化mstatus（spp，spie，mie，sie）     
	csrs mstatus, t1
	li t1, 0x0a0             #1010 0000   mie的第7、5位置1   mtie && stie
	csrs mie, t1

	# 切换模式
	li t1, 0x800             #t1=1000 0000 0000  supervisor mode = 01
	csrs mstatus, t1         #将mstatus第11位置1
	li t1, 0x1000            #t1=0001 0000 0000 
	csrc mstatus, t1         #将mstatus第12位清0

	# 切换模式
	la t1, _supervisor       # t1=_supervisor
	csrw mepc, t1            # 将t1的值写入mepc（出现异常的返回地址，用mret必须要有的）
	mret                     

_supervisor:
	/*your code*/
	li t0,0
	csrw  satp, t0   # 设置`satp`寄存器为0，暂时关闭MMU
	li s1,0xffffffe000000000
	li t2,0x80000000
	sub s1,s1,t2 # -0x8000 0000 
	# offset store in t3 
	#  设置`stvec`为异常处理函数`trap_s`在**虚拟地址空间下**的地址
	la t1, trap_s         
	add t1,t1,s1
	csrw stvec, t1           
	# 记录`start_kernel`在**虚拟地址空间下**的地址，加载到寄存器t4中
	la t1, start_kernel
	add s4, s1,t1 
	la sp,stack_top 
	# 设置`sp`的值为`stack_top`的物理地址（设置低地址空间栈环境）
	call paging_init  
	# 调用`paging_init`函数进行映射 # 设置`satp`的值以打开MMU
	li t2, 1
	slli t2,t2,63
	la s3, _end
	srli s3, s3, 12
	add t1,s3,t2  
	csrw satp, t1  
	# 执行`sfence.vma`指令同步虚拟内存相关映射
	sfence.vma 
	# 设置`sp`的值为虚拟地址空间下的`init_stack_top`
	la sp, init_stack_top
	add sp, sp, s1
	jr s4 #  jalr rd,rs1,offset ,  Jump to address and place return address in rd x4.
	
_mtrap:
	# 交换mscratch和sp, 使用上下文空间作为`trap_m`的栈并保存`x1-x31`寄存器。
	csrrw sp,mscratch,sp

	# 保存寄存器
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
	sd t5, -240(sp)
	sd t6, -248(sp)
	csrr t0, mstatus
	csrr t1, mepc
	sd t0, -256(sp)        	 # 保存mstatus
	sd t1, -264(sp)	         # 保存mepc
	addi sp, sp, -264

	# 通过检查mcause，来判断为何种异常
	csrr t0,mcause
	srai t2, t0, 63          # 算术右移63位
	bnez t2, is_int          # 若非0，则跳往is_int

	#判断是不是environment call from S-mode
	li t1, 9	           
	beq t0, t1, encall_from_s
	j other_trap	         #其他异常

	#判断是不是machine timer interrupt
is_int:
	andi t0, t0, 0x7ff	     #去掉最高位
	li t1, 7    
	beq	t0, t1, time_interupt
	j other_trap


time_interupt:
	# 禁用时钟中断
	li t1, 0x80              #1000 0000
	csrc mie, t1             #对应位清0

	# 设置stip为1
	li t1, 0x20              #10 0000
	csrs mip, t1           

	# 恢复寄存器mepc和mstatus
	ld t0, 0(sp)
	ld t1, 8(sp)
    csrw mepc, t0 	         #恢复寄存器mepc
    csrw mstatus, t1 	     #恢复寄存器mstatus
	j exit

encall_from_s:
	# mtimecmp+=fff000
	li t1, 0x2004000         #t1=mtimecmp的地址
	ld t0, 0(t1)             #读mtime的8字节的内容到t0中
	li t3, 0xfff000
	add t0, t0, t3           #a0加上fff000
	sd t0, 0(t1)             #将t0的值存入地址为t1的内存中（8字节）

	# 清stip
	li t1, 0x20              #10 0000
	csrc mip, t1

	# 使能时钟中断
	li t1, 0x80              #1000 0000
	csrs mie, t1             #对应位置1

	# 恢复寄存器mepc和mstatus
	ld t0, 0(sp)
	addi t0, t0, 4           #mepc+4
	ld t1, 8(sp)
    csrw mepc, t0 	         #恢复寄存器mepc
    csrw mstatus, t1 	     #恢复寄存器mstatus
	j exit


other_trap:
	# 恢复寄存器mepc和mstatus
	ld t0, 0(sp)
	ld t1, 8(sp)
    csrw mepc, t0 	         #恢复寄存器mepc
    csrw mstatus, t1 	     #恢复寄存器mstatus

exit:
	#恢复寄存器
    ld t6, 16(sp)
    ld t5, 24(sp)
    ld t4, 32(sp)
    ld t3, 40(sp)
    ld s11, 48(sp)
    ld s10, 56(sp)
    ld s9, 64(sp)
    ld s8, 72(sp)
    ld s7, 80(sp)
    ld s6, 88(sp)
    ld s5, 96(sp)
    ld s4, 104(sp)
    ld s3, 112(sp)
    ld s2, 120(sp)
    ld a7, 128(sp)
    ld a6, 136(sp)
    ld a5, 144(sp)
    ld a4, 152(sp)
    ld a3, 160(sp)
    ld a2, 168(sp)
    ld a1, 176(sp)
    ld a0, 184(sp)
    ld s1, 192(sp)
    ld s0, 200(sp)
    ld t2, 208(sp)
    ld t1, 216(sp)
    ld t0, 224(sp)
    ld tp, 232(sp)
    ld gp, 240(sp)
    ld ra, 248(sp)
	ld sp, 256(sp)

	# 在`trap_m`返回前将`mscratch`与`sp`寄存器的值重新交换回来。
	csrrw sp,mscratch,sp
	mret
