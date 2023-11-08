



 lab5/6 有意向的同学可以尝试着做一下，相关事项如下：
\1. lab5/lab6要求单人完成
\2. lab5/lab6需要提交实验代码+提交实验报告+完成验收
\3. lab5/lab6建立在lab4上，只提供额外的代码。实验文档是上一届的，没有做修改，可能有一定的阅读难度，在github上提供了上一届risc-v版本的其他文档（/lab20fall-stu）。
\4. lab5/lab6助教不提供答疑。

Lab 5: RISC-V 64 用户模式

## 1 实验目的

进一步巩固课堂学习的页式内存管理以及虚拟内存的相关知识，在低地址空间映射用户态程序，并尝试编写简单的系统调用处理函数。

## 2 实验目标

- 实现用户态进程
- 完成write、getpid系统调用

## 3 实验环境

- Docker Image   `docker exec -it  -u oslab -w /home/oslab reverent_noyce /bin/bash`

## 4 背景知识

### 4.1 User 模式基础介绍

处理器具有两种不同的模式：用户模式和内核模式。在内核模式下，执行代码对底层硬件具有完整且不受限制的访问权限，它可以执行任何CPU指令并引用任何内存地址。在用户模式下，执行代码无法直接访问硬件，必须委托给系统提供的接口才能访问硬件或内存。处理器根据处理器上运行的代码类型在两种模式之间切换。应用程序以用户模式运行，而核心操作系统组件以内核模式运行。

当启动用户模式应用程序时，内核将为该应用程序创建一个进程，为应用程序提供了专用虚拟地址空间等资源。因为应用程序的虚拟地址空间是私有的，所以一个应用程序无法更改属于另一个应用程序的数据。每个应用程序都是独立运行的，如果一个应用程序崩溃，其他应用程序和操作系统不会受到影响。同时，用户模式应用程序可访问的虚拟地址空间也受到限制，在用户模式下无法访问操作系统的虚拟地址，可防止应用程序修改关键操作系统数据。

### 4.2 系统调用约定

系统调用是用户态应用程序请求内核服务的一种方式。在RISC-V中，我们使用 `ecall`指令进行系统调用。当执行这条指令时处理器会提升特权模式，跳转到异常处理函数处理这条系统调用。

Linux中RISC-V相关的系统调用可以在 `.../include/asm-generic/unistd.h`中找到，[syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html)手册页上对RISC-V架构上的调用说明进行了总结，系统调用参数使用a0 - a5，系统调用号使用a7，系统调用的返回值会被保存到a0, a1中。

## 5 实验步骤

### 5.1 环境搭建

#### 5.1.1 建立映射

同lab4的文件夹映射方法，目录名为lab5。 `docker exec -it  -u oslab -w /home/oslab reverent_noyce /bin/bash`

#### 5.1.2 组织文件结构

```
lab5
├── arch
│   └── riscv
│       ├── include
│       │   ├── put.h
│       │   ├── sched.h
│       │   ├── syscall.h
│       │   └── vm.h
│       ├── kernel
│       │   ├── entry.S
│       │   ├── head.S
│       │   ├── Makefile
│       │   ├── sched.c
│       │   ├── strap.c
│       │   ├── vm.c
│       │   └── vmlinux.lds
│       └── Makefile
├── include
│   └── 各个头文件
├── init
│   └──main.c,test.c
├── lib
│   └── put.c  rand.c
└── Makefile
```

要自己写makefile

arch\riscv\Makefile .  编译出 vmlinux,  boot/image,  system.map   PHONY就是无论有没有都执行.

```makefile
.PHONY : all
all : ../../vmlinux boot/image ../../System.map

../../vmlinux : ../../init/main.o ../../init/test.o kernel/head.o kernel/entry.o kernel/strap.o ../../lib/put.o ../../lib/rand.o kernel/sched.o kernel/vm.o
	$(LD) ../../init/main.o ../../init/test.o kernel/head.o kernel/entry.o kernel/strap.o kernel/sched.o kernel/vm.o ../../lib/put.o ../../lib/rand.o -T kernel/vmlinux.lds -o ../../vmlinux

boot/image : ../../vmlinux
	$(OBJCOPY) -O binary ../../vmlinux boot/Image --strip-all

../../System.map : ../../vmlinux
	nm ../../vmlinux > ../../System.map
```

arch\riscv\kernel\Makefile  编译head.S , entry.S,  strap.c,  sched.c, vm.c 

```makefile
.PHONY : all
all : head.o entry.o strap.o sched.o vm.o

head.o : head.S 
	$(GCC) $(CFLAG) head.S

entry.o : entry.S
	$(GCC) $(CFLAG) entry.S

strap.o : strap.c ../include/put.h ../include/syscall.h
	$(GCC) $(CFLAG) strap.c

sched.o : sched.c ../include/sched.h ../include/rand.h
	$(GCC) $(CFLAG) sched.c

vm.o : vm.c ../include/vm.h ../include/put.h
	$(GCC) $(CFLAG) vm.c
```

最顶层的makefile

```makefile
export
CROSS_= riscv64-unknown-elf-
AR=${CROSS_}ar
GCC=${CROSS_}gcc
LD=${CROSS_}ld
OBJCOPY=${CROSS_}objcopy

ISA ?= rv64imafd
ABI ?= lp64
# 这些就直接从之前的实验复制过来。 
INCLUDE = -I ../include
CF = -g -O3 -march=$(ISA) -mabi=$(ABI) -mcmodel=medany -ffunction-sections \
		-fdata-sections -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections
CFLAG = ${CF} ${INCLUDE} -c -DPRIORITY
# cflag  会传递到下面每层
# 定义每个文件夹
RISCV = arch/riscv/
INIT = init/
KERNEL = arch/riscv/kernel/
LIB = lib/
# .PHONY保证不管有没有最新的这些文件， 都会执行下面这些命令。 
.PHONY : all ${LIB} ${INIT} ${KERNEL} ${RISCV} 
all : ${LIB} ${INIT} ${KERNEL} ${RISCV}

${LIB} : 
	make -C ${LIB}
${INIT} :
	make -C ${INIT}
${KERNEL} :
	make -C ${KERNEL}
${RISCV} :
	make -C ${RISCV}

run : 
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux

debug : 
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -S -s

run-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin

debug-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin -S -s 

.PHONY : clean
clean :
	rm vmlinux init/main.o init/test.o ${KERNEL}/head.o ${RISCV}/boot/Image lib/put.o lib/rand.o ${KERNEL}/entry.o ${KERNEL}/strap.o System.map ${KERNEL}/sched.o ${KERNEL}/vm.o 
```

### 5.2 添加系统调用处理函数

本实验我们将增加对系统调用的处理。在RISC-V中系统调用通过 `ecall`（environment call）来实现。在U-mode、S-mode、M-mode下执行 `ecall`分别会触发environment-call-from-U-mode异常、environment-call-from-S-mode异常、environment-call-from-M-mode异常。在系统调用的实现中，我们通过在用户模式U-mode下执行 `ecall`触发environment-call-from-U-mode异常，并由特权模式S-mode中运行的内核处理这个异常。

在head.S中内核boot阶段时，设置 medeleg 寄存器为用户模式系统调用添加异常委托。

在没有设置异常委托的情况下，`ecall`指令产生的异常由M-mode来处理，而不是交由内核所在的S-mode进行处理。通过 medeleg 中设置相应的位，可以将environment-call-from-U-mode异常直接交由S-mode处理。具体设置方式参照RISC-V手册。

为了便于确认实验完成的正确与否，本实验对异常处理函数的命名和类型有要求，要求其函数原型如下：

```c
handler_s(size_t scause, size_t sepc, uintptr_t *regs);
```

此处的regs概念对应linux中的pt_regs，将上下文内容所在的地址传入处理函数。

为S模式异常处理函数添加系统调用的处理逻辑，从a7中获得当前系统调用号，从a0 - a5中获得系统调用相关的参数，本次实验中要求实现的系统调用分别为：172号系统调用 `SYS_GETPID`，64号系统调用 `SYS_WRITE`。

RISC-V中 scause 寄存器储存了中断或异常的类型（原因），通过判断scause的值来进行相应的处理，针对environment-call-from-U-mode异常，从a7中获得系统调用号实现相应的系统调用。系统调用的返回值通过a0，a1寄存器返回。

本次实验要求的系统调用函数原型以及具体功能如下：

- 64  号系统调用`sys_write(unsigned int fd, const char* buf, size_t count)`该调用将用户态传递的字符串打印到屏幕上，此处`fd`为标准输出（1），`buf`为用户需要打印的起始地址，`count`为字符串长度，返回打印的字符数。
- 172 号系统调用`sys_getpid()`
  该调用从`current`中获取当前的`pid`放入`a0`中返回，无参数。

这两个非常短, 就直接写在syscall.h 了

```c
unsigned long sys_write(unsigned int fd, const char* buf, size_t count) {
    if (fd == 1) {
        puts(buf);
        return count;
    }
}

unsigned long sys_getpid() {
    return current->pid;
}
```

```c

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
```

### 5.3 修改进程初始化以及进程调度相关逻辑

#### task struct的修改

本实验中需要为task struct增添新的变量，根据实现方式的不同，新增的方式会有所不同，同学们根据自己之前的实现**按需设计**。举例如下：

```
  size_t sepc;      // 保存的sepc
  size_t sscratch;  // 保存的sscratch
  mm_struct *mm;    // 虚拟内存映射相关
```

在进程初始化时，除了之前实验所需的内容外，还需要为新进程创建页表。新进程页表首先需要把内核相关的映射建立好，这里可以选择简单地把内核页表复制一份（需要注意如何同步不同进程间内核页表的变化）还需要建立用户程序以及用户栈的分配和映射。

本实验还需要储存虚拟内存映射相关的 `mm_struct`，`mm_struct`需要有进程的页表地址、虚拟地址空间的分配情况等信息。

```c
/* 进程数据结构 */
struct task_struct {
     long state;    // 进程状态 Lab3中进程初始化时置为TASK_RUNNING
     long counter;  // 运行剩余时间 
     long priority; // 运行优先级 1最高 5最低
     long blocked;
     long pid;      // 进程标识符
    // Above Size Cost: 40 bytes

    struct thread_struct thread; // 该进程状态段
};
struct mm_struct {
    unsigned long *pgtbl; // pagetable 地址
    unsigned long text_start; 
    unsigned long text_end;
    unsigned long stack_top;
};

/* 进程状态段数据结构 */
struct thread_struct {
    size_t ra;
    size_t sp;
    size_t s0;
    size_t s1;
    size_t s2;
    size_t s3;
    size_t s4;
    size_t s5;
    size_t s6;
    size_t s7;
    size_t s8;
    size_t s9;
    size_t s10;
    size_t s11;
    size_t sepc;
    size_t sscratch;
    struct mm_struct *mm;
};
```

初始化进程后，需要设置好sepc、sstatus等CSR的值，并且需要修改ra使该进程启动后能进行 `sret`，跳转到用户态程序入口。 

<!-- 与进程切换不同，进程初始化后，当其第一次被调用时，该线程的内核栈上缺少陷入中断的上下文信息，因此需要实现一段汇编进行进程的返回。-->

#### 内存空间分配

由于增加了用户态支持，虚拟地址的映射方式与lab4有所不同，变化如下。虚拟地址0x0-0x80000000供用户态使用，0xffffffdf80000000-0xffffffe000000000作为设备地址（UART等）、以及内核用于进行虚拟地址映射的地址，0xffffffe000000000开始的地址直接与物理内存进行映射。

```
Physical Address
----------------------------------------------------------
|      |UART|              |   Kernel 16MB   |
----------------------------------------------------------
       ^                   ^
    0x10000000           0x80000000
       |                   └─────────────────────────────────────────────────────┐
       └────────────────────────────────────────────────┐                        |
Virtual Address                                         ↓                        ↓
------------------------------------------------------------------------------------------------------------
| User 2GB |                      <-- User Stack |      |UART|                   |   Kernel 16MB   |
------------------------------------------------------------------------------------------------------------
^          ^                                     ^      ^                        ^
0x0   0x80000000                0xffffffdf80000000      0xffffffdf90000000       0xffffffe000000000
```

```c

struct kernel_mem_struct {
    unsigned long text_start, rodata_start, data_start;
    unsigned long virtual_offset;
};
//调用creat_mapping函数来建立虚地址映射
void initial_pgtbl(unsigned long pgtbl) {
    unsigned long text_start = kmem_struct.text_start, rodata_start = kmem_struct.rodata_start, data_start = kmem_struct.data_start;
    unsigned long virtual_offset = kmem_struct.virtual_offset;
    create_mapping(pgtbl, virtual_offset + text_start, text_start, rodata_start - text_start, 5);
    create_mapping(pgtbl, virtual_offset + rodata_start, rodata_start, data_start - rodata_start, 1);
    create_mapping(pgtbl, virtual_offset + data_start, data_start, 0x1000000 - (data_start - text_start), 3);

    create_mapping(pgtbl, 0xffffffdf90000000, 0x10000000, 0x1000, 3);

    static unsigned long stack_page_top = 0x84001000;
    create_mapping(pgtbl, 0xffffffdf80000000 - 0x1000, stack_page_top, 0x1000, 11);
    stack_page_top += 0x1000;
    create_mapping(pgtbl, 0x0, 0x84000000ul, 0x1000, 15);
}
```

#### 用户栈与内核栈

用户态程序与内核并不共用栈，因此内核的实现需要区分用户态栈和内核态栈，这就带来一个问题，即在异常处理的过程中需要对栈进行切换。

从用户态进入内核态时，即发生中断或异常时，需要切换用户栈为内核栈，一种实现方式是，访问进程控制结构中的内核栈数据，我们可以使用sscratch来保存当前进程task struct的地址。为实现这一功能，当异常发生时需要首先取出sscratch的内容，方便在后续的逻辑中直接访问task struct中的内容，同样的，在退出异常时，需要再将task struct的地址存回到sscratch中。

另外一种思路是，在用户态下，使用sscratch来直接保存指向内核栈的指针，使用逻辑与上面类似，不再赘述。同时，在进程的调度过程中，同学们也需要根据自己的实现为进程的sscratch、sepc、scause等必要的特权寄存器内容进行切换。

使用`sscratch`保存内核栈顶，在进入trap_s后与`sp`交换。之后使用内核栈。

```assembly
trap_s:
    csrrw sp, sscratch, sp
# 使用sscratch来保存当前进程task struct的地址
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

    csrr a0, scause
    csrr a1, sepc
    add a2, sp, x0

    jal x1, handler_s

    skip_strap:
    ecall
```

sched.c

```c
    //在进程迁移的时候对`sscratch`、`sepc`和`satp`进行设置。
void switch_to_asm(struct task_struct* next) {
    __asm__ __volatile__("csrr t0, sepc\n\tadd %0, t0, x0\n\t"
		:"=r"(current->thread.sepc)
		);
    __asm__ __volatile__("csrr t0, sscratch\n\tadd %0, t0, x0\n\t"
		:"=r"(current->thread.sscratch)
		);
...
    __asm__ __volatile__("add t0, %0, x0\n\tcsrrw x0, sepc, t0\n\t"
		: :"r"(next->thread.sepc)
		);
    __asm__ __volatile__("add t0, %0, x0\n\tcsrrw x0, sscratch, t0\n\t"
		: :"r"(next->thread.sscratch)
		);
    __asm__ __volatile__("addi t0, t0, 1\n\tslli t0, t0, 63\n\tsrl t1, %0, 12\n\tor t0, t0, t1\n\tcsrrw x0, satp, t0\n\t"
		: :"r"(next->thread.mm->pgtbl)
		);
}
```

### 5.4 用户态测试程序

由于内核和用户态程序的功能不同，以及为了兼容将来文件系统等功能，本次实验中所使用的用户态程序需要额外进行编译。同学们在实验的过程中可以选择自行编译用户态程序或者使用仓库中提供的二进制镜像。下面对加载二进制镜像的操作进行讲解，我们使用qemu的initrd选项来加载用户态程序。这里实验仓库中提供的用户态程序是一段经过处理的二进制镜像，同学们在使用下面的命令加载时，实际上是把该段镜像以initfs的形式加载到内存0x84000000处，那么只需要在运行时将此页代码映射至用户空间即可：

```
$ qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin
```

这个可以写在 makefile 中。 

在Makefile中增加命令

```makefile
run-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin

debug-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin -S -s 
```

`hello.bin` 未经裁切前的内容如下所示，可以看到测试程序在main中执行了一次172号调用来获得进程pid，程序在printf中还调用了64号系统调用来打印字符串。

```
0000000000000004 <main>:
   4:   fd010113                addi    sp,sp,-48
   8:   02113423                sd      ra,40(sp)
   c:   02813023                sd      s0,32(sp)
  10:   00913c23                sd      s1,24(sp)
  14:   01213823                sd      s2,16(sp)
  18:   01313423                sd      s3,8(sp)
  1c:   00010993                mv      s3,sp
  20:   00000917                auipc   s2,0x0
  24:   3b890913                addi    s2,s2,952 # 3d8 <printf+0x358>
  28:   fff00493                li      s1,-1
  2c:   0ac00893                li      a7,172
  30:   00000073                ecall
  34:   00050413                mv      s0,a0
  38:   00098613                mv      a2,s3
  3c:   00040593                mv      a1,s0
  40:   00090513                mv      a0,s2
  44:   03c000ef                jal     ra,80 <printf>
  48:   00048793                mv      a5,s1
  4c:   fff7879b                addiw   a5,a5,-1
  50:   fe079ee3                bnez    a5,4c <main+0x48>
  54:   fd9ff06f                j       2c <main+0x28>

000000000000004c <printf>:
  ...
 380:   04000893                li      a7,64
 384:   00078513                li      a0,1
 388:   00070593                mv      a1,a4 # buf
 38c:   00068613                mv      a2,a3 # count
 390:   00000073                ecall
```

### 5.5 编译及测试

```
ZJU OS LAB 5             STUDEDNT-ID/GROUP-ID
[PID = 1] Process Create Successfully! counter = 5
[PID = 2] Process Create Successfully! counter = 5
[PID = 3] Process Create Successfully! counter = 5
[PID = 4] Process Create Successfully! counter = 5
[!] Switch from task 0[0xffffffe000006030] to task 4[0xffffffe000006310], prio: 5, counter: 5
[User] pid: 4
[!] Switch from task 4[0xffffffe000006310] to task 3[0xffffffe000006258], prio: 5, counter: 5
[User] pid: 3
[!] Switch from task 3[0xffffffe000006258] to task 2[0xffffffe0000061a0], prio: 5, counter: 5
[User] pid: 2
[!] Switch from task 2[0xffffffe0000061a0] to task 1[0xffffffe0000060e8], prio: 5, counter: 5
[User] pid: 1
```

![image-20211226214200779](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211226214200779.png)

## 6 实验任务与要求

  请仔细阅读背景知识，理解如何建立页表映射，并按照实验步骤完成实验，撰写实验报告，需提交实验报告以及整个工程的压缩包。

- 实验报告：
  * 各实验步骤截图以及结果分析
  * 实验结束后心得体会
  * 对实验指导的建议（可选）
- 工程文件
  * 所有source code（执行过make clean的工程，不需要提交中间及结果文件）

```
lab5_319010XXXX
├── arch
│   └── ...
├── include
│   └── ...
├── init
│   └── ...
├── lib
│   └── ...
└── Makefile
```

### 心得体会

 本次实验在之前线程调度的基础上增加了虚拟地址，使得之前实验的很多疑问都得到了解决。比如在task_struct中增加几位数据可以解决许多之前实验只能放在栈中保存的数据。 学习了很多写makefile 的范式，  进一步学习了 进程的数据结构， 里面还包含页表地址， 保存寄存器， 还有虚拟内存映射的 地址空间分配情况。  学习了怎么跳转到用户态， 对用户态的栈切换。 基本思想就是保存寄存器 ， 跳转回去。 

初始化一个进程, 一开始要映射虚拟地址, 所以需要多次调用create mapping. 

main就是*unsigned* *int* i = 0; i < 0xFFFFFFFF; i++)  过一段时间就输出 [user] pid, 但是为什么他之前是每次switch都会输出 , 4次之后他就不会printf了.  不过实例中也就printf了四次. 

卡住时间比较久的bug：在创建页表时直接使用了字面量`0x84000000`传入`create_mapping`函数，改值先被解析为int类型，后传入参数里的`unsigned long`类型，导致错误。经验总结：数字字面量的时候要使用类型对应的后缀（比如unsigned int为\*u，unsigned long为\*ul）

建议：

感觉指导中要做的事情还是不是很明确， 比如用户栈与内核栈好像讲了一些， 但是不知道总共有几个点。 要求和讲解混淆在一起了。  
