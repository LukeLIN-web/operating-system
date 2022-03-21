# Lab 4: RISC-V 虚拟内存管理

## 1 实验目的

结合课堂学习的页式内存管理以及虚拟内存的相关知识，尝试在已有的程序上开启 MMU 并实现页映射，保证之前的进程调度能在虚拟内存下正常运行。

## 2 实验内容及要求

* 实现Sv39分配方案下的三级页表映射
* 了解页表映射的权限机制，设置不同的页表映射权限，完成对section的保护及相应的异常处理。

**【！！注意！！】本次实验由于涉及MMU机制较为复杂，考虑之下取消了OpenSBI相关内容，直接使用汇编语言实现制造时钟中断，同学们需要先整体看一遍提供的代码，可以结合之前学习的知识以及代码中的注释理解该过程。**

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，跟随实验步骤完成实验，以截图的方式记录命令行的输入与输出，注意进行适当的代码注释。如有需要，请对每一步的命令以及结果进行必要的解释。**本实验将进行查重，请提供一定的代码注释。**

## 3 实验步骤 

### 3.1 实验背景

#### 3.1.1 建立映射

同lab3的文件夹映射方法，目录名为lab4。

```shell
docker run -it -v /c/Users/12638/Desktop/operatingSystem/lab/lab4:/home/oslab/lab4 -u oslab -w /home/oslab eb33 /bin/bash
 docker exec -it  -u oslab -w /home/oslab reverent_noyce /bin/bash
```

#### 3.1.2 组织文件结构

```
lab4
├── arch
│   └── riscv
│       ├── boot
│       ├── include
│       │   ├── put.h
│       │   ├── sched.h
│       │   └── vm.h
│       ├── kernel
│       │   ├── entry.S
│       │   ├── head.S
│       │   ├── Makefile
│       │   ├── sched.c
│       │   ├── strap.c
│       │   ├── vm.c
│       │   └── vmlinux.lds
│       └── Makefile
├── include
│   ├── put.h
│   └── rand.h
├── init
│   ├── main.c
│   └── Makefile
├── lib
│   ├── Makefile
│   ├── put.c
│   └── rand.c
└── Makefile
```

**本次实验中将设置satp寄存器开启Sv39内存分配方案，随后设置物理地址与虚拟地址的映射，最后为不同section设置不同的映射权限，并实现相应的异常处理。为了实现映射，同学们需要分配内存空间给页表，并对页表项进行相应的设置。由于开启了MMU，还需要对进程调度的相关代码进行一定的修改。**

本次实验由于涉及MMU机制较为复杂，考虑之下取消了OpenSBI相关内容，直接使用汇编语言实现时钟中断，同学们需要先整体看一遍提供的代码，可以结合之前学习的知识以及代码中的注释理解该过程。

#### 3.1.3 Sv39分配方案

本次实验使用Sv39分配方案，支持三级页表映射，请首先阅读**【附录A.虚拟内存及分页机制】**、**【附录B. RISC-V Sv39 分页方案】**中的内容，确保自己已经理解了相关内容，再进行后续实验。

### 3.2 修改head.S（30%）

#### 3.2.1 修改系统启动部分代码

**提示：**`vmlinux.lds`中规定将内核放在物理内存`0x80000000`、虚拟内存`0xffffffe000000000`的位置，因此物理地址空间下的地址x在虚拟地址空间下的地址为`x-0x8000 0000+0xffff ffe0 0000 0000`。

**请严格按照下述顺序进行编写！**

- 在`_supervisor`开头先设置`satp`寄存器为0，暂时关闭MMU

- 设置`stvec`为异常处理函数`trap_s`在**虚拟地址空间下**的地址

- 记录`start_kernel`在**虚拟地址空间下**的地址，加载到寄存器中

   sub 8 左移7 位,   add ffff ffe  左移  .其实可以直接 li 8000 0000 .   哦不对这是16进制!  所以是  ffff ffe 左移36位 

- 设置`sp`的值为`stack_top`的物理地址（设置低地址空间栈环境）

- 调用`paging_init`函数进行映射

- 设置`satp`的值以打开MMU，注意修改`satp`后需要执行`sfence.vma`指令同步虚拟内存相关映射

- 设置`sp`的值为虚拟地址空间下的`init_stack_top`

- 使用`jr`伪指令跳转到虚拟地址下的`start_kernel`（读取之前记录在寄存器中的值），并在虚拟地址空间中执行后续语句与进程调度 

```asm
_supervisor:
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
```

补充一个问题，在head.S里写汇编代码的时候，注意一下使用的寄存器，我们在_surpervisor里涉及了call paging_init这个函数调用，一些需要保留的值建议用s0~s7保存。
相关：寄存器分为调用者保存和被调用者保存，调用前后，调用者保存的寄存器（参数寄存器$a0~$a3和临时寄存器$t0~$t7）可能会有变动，需要在调用函数的函数里自行手动保存。对于一些需要调用函数前后保持不变的数值，如果你不想写保存和恢复寄存器的代码，可以用被调用者保存寄存器保存这些值（保存寄存器 $s0~$s7和返回地址寄存器$ra）。

<u>**Q1：satp的值应当设置为?(提示：映射时将从`_end`开始为页表分配物理空间)**</u>

答： 最高位 =1 , 后面加上end 

satp 有三个域。MODE 域可以开启分页并选择页表级数，图 10.13 展示了它的编码。ASID（Address Space Identifier， 地址空间标识符）域是可选的，它可以用来降低上下文切换的开销。最后，PPN 字段保存了根页表的物理地址，它以 4 KiB 的页面大小为单位

mode  = 8   ASID置零.   _end 的地址是多少? 就是li t1 _end ,  0x8  左移 63位然后 加上end  . 这里获得的是物理地址, 所以需要右移12位. 

Sv39 的转换过程几乎和 Sv32 相同，区别在于具有较大的 PTE 和更多级页表, 就是比图10.14 的sv32多 一层页表, 从32位到43位  根页表在哪里?   PN = physical address >> 12  

虚拟内存屏障(Fence Virtual Memory). R-type, RV32I and RV64I 特权指令。 根据后续的虚拟地址翻译对之前的页表存入进行排序。当 rs2=0 时，所有地址空间的翻译都 会受到影响；否则，仅对 x[rs2]标识的地址空间的翻译进行排序。当 rs1=0 时，对所选地址 空间中的所有虚拟地址的翻译进行排序；否则，仅对其中包含虚拟地址 x[rs1]的页面地址翻译进行排序。

#### 3.2.2 修改M模式下异常处理代码

该小节代码已提供，仅需理解即可。

- 由于M模式下依然使用物理地址，使用虚拟地址将导致内存访问错误。因此，需要保留一片物理地址区域用于异常处理前保存所有寄存器的值。
- `mscratch`寄存器是M mode下专用的临时寄存器。通常，它就用于保存M mode下上下文物理空间的地址。lds文件中分配出了1个page的空间用于储存进程上下文，其顶部标记为`stack_top`，在head.S进入S mode之前的适当位置，将`mscratch`寄存器设置为`stack_top`的物理地址。
- 在M mode异常处理函数`trap_m`的开头，将`mscratch`与`sp`寄存器的值交换（hint: 使用`csrrw`指令），使用上下文空间作为`trap_m`的栈并保存`x1-x31`寄存器。
- 在`trap_m`返回前将`mscratch`与`sp`寄存器的值重新交换回来。

### 3.3 实现映射机制（40%）

#### 3.3.1 创建映射（30%）

为了增加一个虚拟地址到物理地址的映射，根据Rv39分配方案中的地址翻译规则，首先需要**根据根页表基地址以及虚拟地址找到相应页表项的地址，并在该过程中为页表分配物理页面，随后根据物理地址及映射的权限设置页表项具体的值**，使得后续MMU工作时能够根据根页表基地址和虚拟地址得到正确的物理地址。

在 `vm.c`中编写函数 `create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm)`，用作页表映射的统一接口，其中参数作用如下：

- `pgtbl`为根页表的基地址
- `va`,`pa`分别为需要映射的虚拟、物理地址的基地址
- `sz`为映射的大小
- `perm`为映射的读写权限

对于多级页表，父节点页表项存储的值是第一个子节点的位置,而同一个节点的子节点是连续存储的，理论上只需要根节点我们就能得到所有页表项的地址。因此可以进行**按需分配**，当需要使用该虚拟地址对应的物理地址时，再分配内存给相应的页表项。若需要为页表空间分配物理页，可以自由管理分配`_end`地址之后的物理内存（提示：一个页面占4KB，如`&_end+0x1000*page_count`）

```C
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
    uint64_t VPN0, VPN1, VPN2;  //three parts of VPN
    uint64_t valid = 1;  //last bit of page table entry 
    uint64_t count = 0;  //count total page table entry size
    uint64_t *pgtbl_1;  //the 2nd page table
    uint64_t *pgtbl_0;  //the 3th page table
    VPN0 = (va >> 30) & 0x1ff;  
    pgtbl[VPN0] = ((uint64_t)(pgtbl_1) >> addr_offset << 10) + 1;
    //first level page table entry stores the base address of the 2nd page table
    while (sz > 0)  //mapping the whole size
    {
        if (!(count & 0x1ff)) //check if the 1st table has been mapped
        {
            if (!(count & 0xfff))  //check if the 2nd table has been mapped
            {
                current_addr += page_size; //get a new unused addresss
                pgtbl_1 = (uint64_t *)current_addr;  //locate the 2nd page table to the new address
                pgtbl[VPN0] = ((uint64_t)(pgtbl_1) >> addr_offset << 10) + valid; //fill the page table entry
            }
            VPN1 = (va >> 21) & 0x1ff;  //use the same method to build the next level page table
            current_addr += page_size;
            pgtbl_0 = (uint64_t *)current_addr;
            pgtbl_1[VPN1] = ((uint64_t)(pgtbl_0) >> addr_offset << 10) + valid;
        }
        
        VPN2 = (va >> 12) & 0x1ff;   
        pgtbl_0[VPN2] = (pa >> addr_offset << 10) + (perm << 1) + valid;
        //the last level page table entry stores the PPN and the perm
        pa += page_size;  
        va += page_size;
        sz -= page_size;
        //move to the next page
        count++;
    }
}
```

<u>**Q2：解释`create_mapping`的代码设计思路。**</u>

**答：**  首先根据传入映射的大小以及页的大小来判断要做映射的次数，而后通过移位操作分别取出传入va的VPN[0~2]以及pa的PPN,用来给后续的页表项赋值。首先判断传入的基地址的VPN2的位置是否有对应的页表项，如果没有就新建一个页，将新页的地址赋给此处页表项的值（注意移位操作把值赋给正确的位置），如果有，根据此页表项的内容给第二级页表的基地址赋值。对第二级页表进行相同的操作，把二级页表的对应页表项赋值并得到第三级页表的基地址。找到第三级页表的对应页表项，把PPN赋值给他。完成映射的建立。

#### 3.3.2 设置映射（10%）

在 `vm.c`中编写 `paging_init`函数，`perm`映射权限可设为7。

* 调用 `create_mapping`函数将内核起始（`0x80000000`）的16MB空间映射到高地址（以 `0xffffffe0 0000 0000`为起始地址）。
* 对内核起始地址（`0x8000 0000`）的16MB空间做等值映射。
* 将必要的硬件地址（即`0x10000000`为起始地址的UART）进行等值映射，无偏移。

具体映射方式可参考【附录D.映射图示】。

```C
void paging_init()
{
    uint64_t *pgtbl = &_end;
    current_addr = (uint64_t)&_end;
    create_mapping(pgtbl, 0x80000000, 0x80000000, 0x1000000, 7); 
    create_mapping(pgtbl, 0xffffffe000000000, 0x80000000, 0x1000000, 7); 
    create_mapping(pgtbl, 0x10000000, 0x10000000, 0x1000, 7); 
}
```

**<u>Q3：为什么要进行等值映射？（提示：不同模式下使用虚拟地址/物理地址）</u>**

**答：**  S模式  一对一映射, M 模式,  虚拟地址. 机器模式下使用等值映射的虚拟地址，用户模式下使用非等值映射的虚拟地址

### 3.4 修改进程调度相关代码`sched.c`（10%）

#### 3.4.1 必要修改

* 将各进程初始化时的`counter`和`priority`设置为相应数组中的值。

  ```c
  task[i]->counter = COUNTER_INIT_COUNTER[i];
  task[i]->priority = PRIORITY_INIT_COUNTER[i];
  ```

* 修改所有进程均处理完毕情况下的操作：在lab3中我们为task0分配一个时间片以重设各task，在本实验中，当所有进程均运行完毕的情况下，利用`rand()`函数随机为**task[1-4]**的`counter`分配一个数值，并再次调用`schedule()`根据调度算法得到下一个需要运行的task。

* 无法使用lab3中利用openSBI完成的打印函数，需要用现有的打印函数对此做更新。

* 本次实验中仅需要编写SJF下的进程调度。其他未提及的修改自行根据代码进行调整。

#### 3.4.2 修改task_init()调整为虚拟地址

由于开启了MMU，因此我们需要修改进程初始化中涉及地址的内容，将进程的地址以及进程结构体成员变量`sp`栈指针的值划分到虚拟地址空间（提示：未分配的空闲物理地址转换为虚拟地址）。将相关代码写入下方代码框。

```c
    task[0] = current;
    task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;
    task[i] = tmp;
    task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
```

### 3.5 完成对不同section的保护（20%）

**【注意】请在完成3.1~3.4，使得实验能够成功运行后再进行这一步。本节中需要【修改】内核起始地址到高地址的映射以及等值映射，即本节中的内容会替换掉3.3.2中的前两个映射。**

#### 3.5.1 权限保护

修改内核起始地址到高地址的映射以及等值映射，对其中不同的段执行不同的权限保护。通过修改调用`create_mapping`时的`perm`参数，修改对内核空间不同section所在页属性的设置，完成对不同section的保护，其中text段的权限为 `r-x`, rodata段为`r--`, 其他段为 `rw-`。 

参考`vmliunx.lds`中对地址的分配，使用相应的标签（如`(uint64_t)&text_start`）计算不同段的起始物理地址、起始虚拟地址及映射大小，给出修改后的`paging_init()`。

|X|W|R|V|     text 段就是  1011 = 0xB   rodata 就是  0011    其他段就是   0111 =0x7

vmliunx.lds 可以知道 , 先 text , 然后 rodata , 然后 data  ,  最后 bss

```c
void paging_init()
{
    uint64_t *pgtbl = &_end;
    current_addr = (uint64_t)&_end;
    // create_mapping(pgtbl, 0x80000000, 0x80000000, 0x1000000, 7); 
    // create_mapping(pgtbl, 0xffffffe000000000, 0x80000000, 0x1000000, 7); 
    // create_mapping(pgtbl, 0x10000000, 0x10000000, 0x1000, 7); 
    
    create_mapping(pgtbl,0xffffffe000000000,0x80000000,0x2000,11);// text
    create_mapping(pgtbl,0xffffffe000002000,0x80002000,0x1000,3);// rodata 
    create_mapping(pgtbl,0xffffffe000003000,0x80003000,0x1000000 - 0x3000,7);//else
    create_mapping(pgtbl,0x10000000,0x10000000, 0x1000,7);// UART
}

```

#### 3.5.2思考题：完成权限保护相关的异常处理并进行测试（不作要求，附加5%）

* 在`head.S`中，通过修改`medeleg`寄存器，将instruction/load/store page fault托管到S模式下

* 修改`strap.c`中的`handler_s`，添加对page fault的打印，注意异常发生后sepc需要更新（见lab1）。

* 在代码中加入对rodata段的写操作和对data段的执行操作，验证权限保护是否完成。

**注：感兴趣的同学可以完成本题，附上相关的代码及说明，思考题满分五分，可以抵扣本实验中其他部分被扣的分数，但不会超过本次实验100分的分数上限。**

```assembly
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

	# 设置将instruction/load/store page fault托管到S模式下
	li t1, 0xb000
    csrs medeleg,t1

	# 初始化.bss
	la t1, bss_start         # t1=bss段start的地址
	la t2, bss_end           # t2=bss段end的地址
	li t3, 0x0               # 填入的内容
```

M 模式还可以通过 medeleg CSR 将同步异常委托给 S 模式。该机制类似于刚才提到的中断委托，但 medeleg 中的位对应的不再是中断，而是图 10.3 中的同步异常编码。例 如，置上 medeleg[15]便会把 store page fault（store 过程中出现的缺页）委托给 S 模式。  置上 medeleg[13]  load      medeleg[12]       instruction 

所以 medeleg  = 1011 0000 0000 0000

```c
void handler_s(uint64_t cause){
	if (cause >> 63) {		// interrupt
		if ( ( (cause << 1) >> 1 ) == 5 ) {	// supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
			count++;
		}
	}
    // 如果最高位为0  就是fault 的处理
	else{
		if(cause == 0xd)
            puts(" load page fault!\n");
		else if(cause == 0xf)
            puts(" store page fault!\n");
        else if(cause == 0xc)
            puts(" instruction page fault!\n");
	}
}
```

怎么写 rodata?

直接写是通不过编译, 需要内嵌汇编. 就是定义const全局变量, 然后内嵌汇编修改

data执行操作, 需要汇编jump到data段,  执行操作

### 3.6编译及测试

仿照lab3进行编译及调试，对`main.c`做修改，确保输出本组成员的学号与姓名，只需要实验SJF模式下的进程调度，需要附上成功运行两轮及以上的截图，**请修改`COUNTER_INIT_COUNTER`数组中的值以区别于示例截图中的{1,2,3,4}。**

![image-20211206220256074](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211206220256074.png)

![image-20211206220303682](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211206220303682.png)

## 4 讨论和心得

请在此处填写实验过程中遇到的问题及相应的解决方式。

由于本实验为新实验，可能存在不足之处，欢迎同学们对本实验提出建议。

1. satp 是几位的

Sv39 使用和 Sv32 相同的 4 KiB 大的基页, 64位的satp

2. 直接取地址是物理地址还是虚拟地址?

应该是物理地址,  因为下面   la t1, stack_top , csrw mscratch, t1是保持上下文物理空间的地址

注意：在未分页时访问的地址都是物理地址。分页后，监管者模式和用户模式下访问的地址是虚拟地址。

3. satp的值应当设置为?

不知道3级页表, PPN 应该怎么设置. 

我们是要 根据根页表基地址以及虚拟地址找到相应页表项的地址，根据物理地址及映射的权限设置页表项具体的值。即已知satp, va, pa, 求相应的页表项地址，并根据物理地址设置页表项具体的值。

4. 求虚拟地址. 

利用伪指令很方便 , 比如li 可以把0x80000000 , 就不用  addi t2,x0, 8  这么麻烦  slli t2, 7 . 是16进制所以看起来 差两个其实需要左移4倍

5. sfence.vma 怎么调用?    设置`satp`的值以打开MMU，注意修改`satp`后需要执行`sfence.vma`指令同步虚拟内存相关映射 怎么做? 

就直接执行命令就行了

#### debug 

`riscv64-unknown-linux-gnu-gdb lab4/vmlinux   target remote localhost:1234`
①先检查自己映射的范围对不对，地址写错没有
②检查自己的映射写的对不对，在paging_init之后模仿MMU走一遍va+pgtbl得到pa的过程 看pa是否正确 
③检查映射的权限对不对 注意下perm只是RWX位 不包含V位 
④不要在create_mapping里打印太多东西，可能会因为时间到了触发时钟中断你打印不下去了，或者可以调大一下head.S里时钟中断的时间间隔先保证输出，最后运行的时候恢复时钟中断注释掉这些打印函数

##### 没有输出

b *0x80200000 也不行,    应该是填入  start_kernel 出现了问题

第一个运行的是哪个汇编函数, head.s吗? 

是的,是head.s , 运行 _start 函数 他跑完了start就在这里无限循环了. 也没有, 只是循环比较多而已.  之后会继续运行 clean_loop 函数.  所以应该是 supervised 出错了.

i reg satp  是0 没有问题 ,   stvec设置为       0xffffffe000000308

 start_kernel`在**虚拟地址空间下**的地址 可以存在x3吗? gp 0xffffffe0000004ac       0xffffffe0000004ac <start_kernel>

当 hart 的时间比较器（一个名为 mtimecmp 的内存映射寄存器）大于实时计数器 mtime 时，会触发时钟中断。

##### 不能跳转到start

b *0x80000050  然后继续往后一步步调试. 

 b *0x800000b8  这里是  supervisor 的地址.   

先加后减 , 不然可能< 0 了

汇编实在是 ,太难调试了. 就很容易出错, 还是C语言好. 

第二轮task 3 到task1 会卡住下不去. 

那记录运行随机值最小的那个,让next等于他 ,就不要递归调用了

`pgtbl_1 = &_end + 0x1000 * page_count;这个输出的和pgtbl_1 = (uint64_t *) ((uint64_t)&_end + 0x1000 * page_count);`   这个输出的 是不一样的

## 附录

### A.虚拟内存及分页机制

#### 1. 虚拟内存

虚拟内存是计算机系统内存管理的一种技术。它使得应用程序认为它拥有连续的可用的内存（一个连续完整的地址空间），而实际上，它通常是被分隔成多个物理内存碎片，还有部分暂时存储在外部磁盘存储器上，在需要时进行数据交换。

通过内存地址虚拟化，可以使得软件在没有访问某虚拟内存地址时不分配具体的物理内存，而只有在实际访问某虚拟内存地址时，操作系统再动态地分配物理内存，建立虚拟内存到物理内存的页映射关系，这种技术称为按需分页（demand paging）。把不经常访问的数据所占的内存空间临时写到硬盘上，这样可以腾出更多的空闲内存空间给经常访问的数据；当CPU访问到不经常访问的数据时，再把这些数据从硬盘读入到内存中，这种技术称为页换入换出（page swap in/out）。这种内存管理技术给了程序员更大的内存“空间”，从而可以让更多的程序在内存中并发运行。

#### 2.MMU

MMU（Memory Management Unit）是一种负责处理中央处理器（CPU）的内存访问请求的计算机硬件。它的功能包括虚拟地址到物理地址的转换（即虚拟内存管理）、内存保护、中央处理器cache的控制。MMU位于处理器内核和连接cache以及物理存储器的总线之间。如果处理器没有MMU，CPU内部执行单元产生的内存地址信号将直接通过地址总线发送到芯片引脚，被内存芯片接收，这就是物理地址。如果MMU存在且启用，CPU执行单元产生的地址信号在发送到内存芯片之前将被MMU截获，这个地址信号称为虚拟地址，MMU会负责把VA翻译成相应的物理地址，然后发到内存芯片地址引脚上。

简而言之，当处理器内核取指令或者存取数据的时候，会提供一个虚拟地址，这个地址是可执行代码在编译的时候由链接器生成的。MMU负责将虚拟地址转换为物理地址，以在物理存储器中访问相应的内容。

#### 3.分页机制

分页机制的基本思想是将程序的虚拟地址空间划分为连续的，等长的虚拟页。虚拟页和物理页的页长固定且相等（一般情况下为4KB），从而操作系统可以方便的为每个程序构造页表，即虚拟页到物理页的映射关系。

逻辑上，该机制下的虚拟地址有两个部分组成：1.虚拟页号；2.页内偏移。在具体的翻译过程中，MMU首先解析得到虚拟地址中的虚拟页号，并通过虚拟页号查找到对应的物理页，用该物理页的起始地址加上页内偏移得到最终的物理地址。

### B. RISC-V Sv39 分页方案

#### A.Sv39分页方案

请阅读**【RISC-V中文手册 10.6节基于页面的虚拟内存】**了解相关知识

S模式提供了一种传统的虚拟内存系统，它将内存划分为固定大小的页来进行地址转换和对内存内容的保护。启用分页的时候，大多数地址（包括 load和 store的有效地址和PC中的地址）都是虚拟地址 。要访问物理内存，它们必须被转换为真正的物理地址 ，这通过遍历一种称为页表的高基数树实现。页表中的叶节点指示虚地址是否已经被映射到了真正的物理页面，如果是，则指示了哪些权限模式和通过哪种类型的访问可以操作这个页。访问未被映射的页或访问权限不足会导致 page fault exception

RV64支持多种分页方案，本次实验使用了Sv39。 Sv39使用4KB大的基页，页表项的大小是8个字节，为了保证页表大小和页面大小一致，树的基数相应地降到$2^9$，树也变为三层。 Sv39的 512 GB地址空间（虚拟地址）划分为$2^9$个 1GB大小的吉页 。每个吉页被进一步划分为$2^9$个2MB大小的巨页。每个巨页再进一步分为$2^9$个4KB大小的基页。 (为什么是 2^9次方呢? )

**注意：在未分页时访问的地址都是物理地址。分页后，监管者模式和用户模式下访问的地址是虚拟地址。**

#### B.  `satp` （Supervisor Address Translation and Protection Register）

satp（Supervisor Address Translation and Protection，监管者地址转换和保护）的 S模式控制状态寄存器控制了分页系统，其内容如下所示：

```c
 63      60 59                  44 43                                0
 ---------------------------------------------------------------------
|   MODE   |         ASID         |                PPN                |
 ---------------------------------------------------------------------
```

* MODE：可以开启分页并选择页表级数，8表示Sv39分配方案，0表示禁用虚拟地址映射。

* ASID (Address Space Identifier) ： 用来区分不同的地址空间，此次实验中直接置0即可。
* PPN (Physical Page Number) ：保存了根页表的物理地址，通常 `PPN = physical address >> 12`。M模式的程序在第一次进入S模式之前会把零写入 satp以禁用分页，然后S模式的程序在初始化页表以后会再次进行satp寄存器的写操作。 

#### C.RISC-V Sv39 Virtual Address and Physical Address

```c
     38        30 29        21 20        12 11                           0
     ---------------------------------------------------------------------
    |   VPN[2]   |   VPN[1]   |   VPN[0]   |          page offset         |
     ---------------------------------------------------------------------
                            Sv39 virtual address
```

```c
 55                30 29        21 20        12 11                           0
 -----------------------------------------------------------------------------
|       PPN[2]       |   PPN[1]   |   PPN[0]   |          page offset         |
 -----------------------------------------------------------------------------
                            Sv39 physical address
```

Sv39模式定义物理地址有56位，虚拟地址有64位。但是，虚拟地址的64位只有39位有效，63-39位在本次实验中（高地址映射）需要为1保证地址有效。Sv39支持三级页表结构，VPN\[2-0](Virtual Page Number)分别代表每级页表的虚拟页号，PPN\[2-0](Physical Page Number)分别代表每级页表的物理页号。物理地址和虚拟地址的低12位表示页内偏移（page offset）(2的12次方为4KB就是一页的大小 .)


#### 1.RISC-V Sv39 Page Table Entry

```c
 63      54 53        28 27        19 18        10 9   8 7 6 5 4 3 2 1 0
 -----------------------------------------------------------------------
| Reserved |   PPN[2]   |   PPN[1]   |   PPN[0]   | RSW |D|A|G|U|X|W|R|V| 
 -----------------------------------------------------------------------
                                                     |   | | | | | | | |
                                                     |   | | | | | | | `---- V - Valid
                                                     |   | | | | | | `------ R - Readable
                                                     |   | | | | | `-------- W - Writable
                                                     |   | | | | `---------- X - Executable
                                                     |   | | | `------------ U - User
                                                     |   | | `-------------- G - Global
                                                     |   | `---------------- A - Accessed
                                                     |   `------------------ D - Dirty (0 in page directory)
                                                     `---------------------- Reserved for supervisor software
```

* 0 ～ 9 bit: protection bits
  * V: 有效位，当 V = 0, 访问该PTE会产生Page fault
  * R: R = 1 该页可读。
  * W: W = 1 该页可写。
  * X: X = 1 该页可执行。
  * U,G,A,D,RSW本次实验中设置为0即可。

#### 2. RISC-V Address Translation Details

![image-20211110144952448](./img/translation.jpg)

虚拟地址翻译为物理地址的完整过程请参考[Virtual Address Translation Process](http://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#sv32algorithm)，建议仔细阅读，简化版内容如下：

* 1.从satp的`PPN`中获取根页表的物理地址。 

* 2.通过pagetable中的VPN段,获取PTE。(可以把pagetable看成一个数组，VPN看成下标。PAGE_SIZE为4KB，PTE为64bit(8B), 所以一页中有4KB/8B=512个PTE，而每级VPN刚好有9位，与512个PTE一一对应)  

*  3.检查PTE的 `V bit`，如果不合法，应该产生page fault异常。

* 4.检查PTE的`RWX`bits,如果全部为0，则从PTE中的PPN[2-0]得到的是下一级页表的物理地址，则回到第二步。否则当前为最后一级页表，PPN[2-0]得到的是最终物理页的地址。

* 5.将得到最终的物理页地址，与偏移地址相加，得到最终的物理地址。
  
* 6.对齐注意 

  >Any level of PTE may be a leaf PTE, so in addition to 4 KiB pages, Sv39 supports 2 MiB megapages and 1 GiB gigapages, each of which must be virtually and physically aligned to a boundary equal to its size. A page-fault exception is raised if the physical address is insufficiently aligned.

  >If i > 0 and pte.ppn[i − 1 : 0] ≠ 0, this is a misaligned superpage; stop and raise a page-fault exception corresponding to the original access type.

**可以自行尝试一遍以下两个过程，确保自己已理解。**

* **【MMU的自动转换】：根据根页表首地址及虚拟地址找到页表项的地址，根据页表项的值转换为物理地址。即已知satp, va, 求相应的页表项地址，并根据页表项的值得到物理地址。**
* **【实验内容】：根据根页表基地址以及虚拟地址找到相应页表项的地址，根据物理地址及映射的权限设置页表项具体的值。即已知satp, va, pa, 求相应的页表项地址，并根据物理地址设置页表项具体的值。**

### C. 关于断点不生效

开启虚拟内存后，会存在一些断点失效的情况，使得无法在C语言源代码层面调试，只能在汇编等级调试。原因是开启MMU后，PC在虚拟的0x80000000空间下，但是GDB中记录的调试信息是在0xfffffe000000000空间下的，因此无法获取调试信息。

助教暂未想到好的方法来解决这个问题，校内论坛中有同学提出可以使PC跳跃到高地址以进行调试，感兴趣的同学可以自行尝试，对于后续bonus的实现也会有一定帮助。

### D. 映射图示
```
Physical Address
  ----------------------------------------------------------
  |      |UART|              |   Kernel 16MB   |
  ----------------------------------------------------------
         ^                   ^
     0x1000 0000           0x8000 0000   
         |                   ├───────────────────────────────────────────────────┐
         |                   |                                                   |
  Virtual↓Address            ↓                                                   ↓
  ---------------------------------------------------------------------------------------------------
         |UART|              |   Kernel 16MB   |                                 |   Kernel 16MB   |
  ---------------------------------------------------------------------------------------------------
         ^                   ^                                                   ^
     0x1000 0000         0x8000 0000                                        0xffffffe0 0000 0000
```
