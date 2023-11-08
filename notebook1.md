[TOC]

# operating system  The first half

Final Exam = 50％ ■ Homework = 5% ■ Quiz = 5% ■ Project Report = 20% ■ Project Face-face check = 20%

quiz 5分, 3次随堂测试. 操作系统用riscv64架构. 后面三个实验两人一组.

lab 0  写两个小时,  编译10分钟.

lab1 写三四条汇编语言. 五六个小时.

lab2-lab4 两个人做,  实现系统中断. 跳转到操作系统态. sbi版本.  lab5选做,  user mode 

### lec1

- Execute user programs and make solving user problems easier.
- 方便易用 -> 要能提供第三方的程序

#### What Operating Systems Do 

####  Computer-System Organization 

OS is a resource allocator 

● Manages all resources
● Decides between conflicting requests for efficient and fair resource use
■ OS is a control program

 ● Controls execution of programs to prevent errors and improper use of  computer   程序运行有bug, 死循环了, 操作系统可以处理.

bootstrap program is loaded at power-up or reboot
● Typically stored in ROM or EPROM, generally known as firmware固件
● Initializes all aspects of system ● Loads operating system kernel and starts execution

I/O devices and CPU can execute concurrently.
■ Each device controller is in charge of a particular device type.
■ Each device controller has **a local buffer.**

 ■ CPU moves data from/to main memory to/from local buffers 

■ I/O is from  device to local buffer of controller. 原来还有buffer来缓冲的. 

■ Device controller informs CPU that it has finished its operation by causing an interrupt (via system bus)

中断是异步的,  interrupt handler 必须保存被中断的执行状态.

trap 是软件产生的中断,   由于 error 或者用户request 引起. trap是同步的.  

#### DMA

 without CPU intervention介入.CPU does not need to check  status register of I/O devices in a busy loop

Cache coherency must be preserved. burst transfer也就是突发传输，是DMA传输模式的一种。DMA的传输模式的以下三种：突发模式(Burst mode)、循环挪用(Cycle stealing mode)模式、透明模式 

突发模式就是在一个连续的时间序列内完成一块完整数据的传输。当CPU将系统总线(AMBA总线)控制权交给DMA控制器，CPU输入空闲状态，直到DMAC将数据块所有数据传输完成并将总线控制权交还给CPU。这个模式叫做块传输模式，也叫做突发模式。

但是DMA时  CPU不能访问内存.

#### Process Management 

Program is a passive entity, process is an active entity.

■ Creating and deleting both user and system processes 

■ Suspending and resuming processes 

■ Providing mechanisms for process synchronization

 ■ Providing mechanisms for process communication 

■ Providing mechanisms for deadlock handling

####  Memory Management 

####  Storage Management

阿里p5-p7是写代码, p8 p 9 写ppt,p10看别人的ppt

win95当场演示蓝屏. 现在稳定多了.win32开始抄袭了苹果的UI.  

### lec2

操作系统structures

syscall 

```c
#include <sys/socket.h> 
int socket(int socket_family, int socket_type, int protocol); 
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen); 
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

编程语言

python依赖于C, 只要做好编译.

不依赖其他语言, 就需要考虑不同的操作系统. 

怎么传递参数给OS?

最简单的是寄存器, 但是寄存器有限

第二个是放在一个block or table.

第三个是放在stack中.

#### Types of System Calls

■ Process control 

■ File management 

■ Device management 

■ Information maintenance (e.g. time, date) 

■ Communications

内存满了, 操作系统要把不太活跃的进程swap到外面去. 

很多人一生不可能设计一个操作系统, 但是这些原理可以用在大型软件中. 设计失败的操作系统, 也是宝贵的经验教训. 

车载系统代码也超过100w行了.但是给用户UI可能是最重要的.

把策略和机制分离出来. it allows maximum flexibility if policy decisions are to be changed later

policy  what will be done?

mechanism: How to do it?

timer for CPU protection is a mechanism

- Do not add new functionality unless an implementer cannot complete a real application without it.
- 听一个两个人的需求 基本上都是错的,  需要广泛调研.
- Provide mechanism rather than policy. In particular, place user interface policy in  clients' hands. 提供机制. 

layer approach

#### 微内核

Moves as much from  kernel into user space 尽量多在用户空间, 比如file system和device driver 都在user mode.

• Implemented as user space program • Mach example of microkernel 

• Mac OS X kernel (Darwin) partly based on Mach

Communication takes place between user modules using message passing 用消息传递

好处: 

Easier to extend a microkernel  容易拓展

• Easier to port  operating system to new architectures 

• More reliable (less code is running in kernel mode) • More secure

自己重新编译内核后/boot中有grub , 这个会考

init 在sbin文件夹中. pid =1 . 配置文件放在/etc/中

### lec3 进程

进程是资源allocation和protection的基本单元. 线程是执行的基本单元

如果IO queue很多 进程, 那么 ready queue  总是空的, short term 就不太需要调度. 

A process includes: 

● text section (code) 

● program counter  PC.
● stack (function parameters, local vars, return addresses)
● data section (global vars) 

● heap (dynamically allocated memory)  Heap is a "complete" binary tree Except  leaf level, all levels are full.  leaf level is filled from left to right
●  node's value should be larger/smaller than its children

#### 进程的状态

● new: 刚被创建

● running:  正在执行指令
● waiting:  process is waiting/blocked for some event to occur. 比如P(wait),申请内存
● ready:  process is waiting to be assigned to a processor.  时间片到了(或者被高优先级进程抢占)就从运行变为就绪状态
● terminated:  结束执行

#### 进程控制块

 Process Control Block (PCB)    包含了Information associated with each process

 ■ Process state (new, ready, ...)  进程状态

■ Program counter (address of next instruction) PC

■ Contents of CPU registers  CPU寄存器

■ CPU scheduling information (priority)  调度信息比如优先级

■ Memory-management information 内存管理信息

■ Accounting information (cpu/time used, time limits, process number) 
■ I/O status information (allocated devices and opened files)  io 状态信息

Linux’s PCB is defined in struct task_struct 这个考过

上下文切换:   OS把P0进程state 保存到 对应的PCB0 中,   然后reload 另一个进程P1的state from PCB1

#### 进程调度

job queue :所有进程

ready queue: ready的进程

device queues 等待IO device的进程

multilevel feedback queue scheduler generally assigns a long quantum时间片 to 低优先级

优先级高的, 分小时间片, 先调度. 如果没运行完, 那就放到下一个queue.

长期 scheduler和短期schelers

长期 scheduler selects which processes should be brought into memory (the ready queue) 然而UNIX and Windows do not use long-term scheduling. 当时想的很好, 后来发现simple is elegant.  进程放在ready 队列

Short-term scheduler (or CPU scheduler) – selects which process should be executed next and allocates CPU 进程分配CPU时间.  

Schedulers (Cont.)
■ Short-term scheduler is invoked very frequently (milliseconds) 所以必须响应很快. 

Need to select good combination of CPU bound and I/O bound processes.

刚完成io, 应该提高优先级尽快处理.  时间片用完, 可以降低优先级, 

#### fork

它有三个返回值
\- 该进程为父进程时，返回子进程的pid
\- 该进程为子进程时，返回0
\- fork执行失败，返回-1

i=0 i<3  fork 会有8个hello打印出来.

**子进程不会执行前面父进程已经执行过的程序，因为PCB中记录了当前进程运行到哪里，而子进程又是完全拷贝过来的，所以PCB的程序计数器也是和父进程相同的，所以是从fork()后面的语句继续执行**。

pthread和thread区别 : pthread是unix的, thread在cpp11有标准实现

 context-switch time is OS overhead.  context-switch overhead 和下面三个因素有关:

 complexity of  OS and PCB  

Multiple sets of registers per CPU

Multiple contexts loaded at once

#### 进程间通讯

1. shared memory共享内存. 内存不在kernel中

2. 信号 Processes must name each other explicitly send (P, message)  ● receive(Q, message) 传给kernel 然后kernel传给另一个进程
3. 消息队列
4. 信号量 ,信号量是一个计数器，用于多进程对共享数据的访问，信号量的意图在于进程间同步。这种通信方式主要用于解决与同步相关的问题并避免竞争条件。
5. 管道
6. 套接字socket 

### lec4 线程

#### 线程的好处

1. 创建的开销小, 进程的数量是有上限的. 
2. 切换的开销小, 不用context switch, 只需要保存寄存器和stack, 保存变量和函数执行到哪里.  
3. 共享同一个进程的资源, 不用进程间通讯, 

线程共享什么资源?   线程共享 进程的地址空间, 堆内存和全局变量

多个user 线程 对应一个kernel 线程  . 优点是线程管理 is efficient, but will block if making system call, kernel can schedule only one thread at a time

一个user线程对应一个kernel线程 One-to-One  ,more concurrency, but creating thread is expensive

多个user线程对应多个kernel线程   Many-to-Many flexible

user 线程切换由应用程序负责, 不用内核参与.

#### **Thread Control Block**

 **TCB** in  kernel contains thread-specific information needed to manage it. Similar to [Process Control Block](https://techaccess.in/2021/05/07/process-control-blockpcb/)(PCB), each thread maintains a table called Thread Control Block(TCB) which contains thread specific information.

**Some of  common attributes kept in TCB are:**

- Thread ID  有tid 
-  content of CPU Register 寄存器组
- Program Counter(PC)  
- Scheduling Information
- [Thread state](https://techaccess.in/2021/05/07/process-states/)(status) 线程状态
- Stack Pointer 栈指针
- Signal Mask
- Thread Parameters like start function, stack size.
- Pointer to PCB of the process of which thread is part of

LWP (Light-weight process) is a virtual processor attached to kernel thread

Switching between user level threads of the same process is often more efficient than switching between kernel threads because: 

 (a) User level threads require tracking less state.  (b) User level threads share the same memory address space.  (c) Mode switching is not necessary. 对的 (d) Execution stays within the same process with user level thread  

a b d kernel和user一样

Pthreads  Is it a user- or kernel-level library? 既可以user也可以kernel因为是C语言

■实验2 Linux 内核重建

内核编程时不能访问C库； ■ 内核编程时必须使用GNU C； ■ 内核编程时缺乏像用户空间那样的内存保护机制； ■ 内核编程时浮点数很难使用； ■ 内核只有很小的定长堆栈； ■ 由于内核支持异步中断、抢占式和SMP，因此必须时刻注意同步和并发；
■ 要考虑可移植性的重要性

有课程作业, 不过比较少, 4-5道题目. 填空选择

lab1 预期7 8个小时,  这个操作系统只能启动, 打印一行字. 

做一个映射, 之后lab也可以都做一个映射

opensbi可以把虚拟机转化为 riscv 汇编. kernel 通过sbi . 去年没有这个, 要多写十几行汇编.  要花三倍时间 .

怎么输出?数字转字符串, 放在指定地址 . 

### lec5 CPU 调度

running -  > wait    and   terminates 是非抢占式

running - > ready  and wait - > ready 是抢占式的.

SJF  最短任务最先调度.  

有两种, 一种是非抢占式,

一种是抢占式, 也叫做 SRTF. Shortest-Remaining-Time-First 每次有一个job进来就会调度它.  这种算法要预测下一次的时间, relies on predicting the next CPU burst based on an average of previous bursts

Shortest Remaining Time First SRTF cannot be implemented because it requires knowledge of the future.

用aging来 防止很长的job starvation.

响应时间: 首次Response的时间.**Response** **time** is the amount of time after which a process gets the **CPU** for the first time after entering the ready queue

wait time: 在就绪队列中等待的时间总和. 

The average turnaround time平均周转时间=作业周转总时间/作业个数 周转时间 =**完成时间 -到达时间**

 结合各种分配方法, 分两个队列, foreground 和back ground 

调度算法的时间每年都会考. 必考. 

操作系统可以用api来改调度算法. 

#### Dispatcher

负责上下文切换, 切换到用户态 module gives control of the CPU to the process selected by the short-term scheduler(调度器来选, dispatch 来给处理器); 

this involves: 

● switching context 

● switching to user mode 

● jumping to the proper location in the user program to restart that program

Dispatch latency is :time it takes for the dispatcher to stop one process and start another running

Dispatch 有啥用? allocate a process to the processor

时间4的时候是运行1 不是3 因为等待队列里是13

### lec6 进程同步

考试考的挺多. 

临界资源

```c
//进入区, 检查是否可以进入, 如果可以, 设置正在访问临界资源的标志来阻止其他进程
//critical section
//exit section
//remainder section
```

**双标志法先检查**

检查对方flag, 修改自己flag . 问题是时间差, 可能都检查通过.

**双标志法后检查**

先把自己flag 修改,然后检查对方flag, 问题是可能starvation

Peterson 算法

先设置自己标志, 然后设置turn 标志. 

```c
flag[i] = true; turn = j;
while(flag[j] && turn == j);//如果 另一个进程Pj已经在临界区, 因为turn =j, 就会不断循环. 
critical section;
flag[i] = false;
remainder section;
```

#### 硬件同步

Atomic = non-interruptable

有 TestAndSet Instruction ,Swap Instruction

#### 同步和互斥的机制

1. **互斥量(Mutex)**：采用互斥对象机制，只有拥有互斥对象的线程才有访问公共资源的权限。因为互斥对象只有一个，所以可以保证公共资源不会被多个线程同时访问。比如 Java 中的 synchronized 关键词和各种 Lock 都是这种机制。
2. **信号量(Semphares)** ：它允许同一时刻多个线程访问同一资源，但是需要控制同一时刻访问此资源的最大线程数量
3. **事件(Event)** :Wait/Notify

#### 信号量

非常重要, 可以说是必考

- P operation also called as wait operation decrements the value of semaphore variable by 1. 也叫做 wait(S)
- V operation also called as signal operation increments the value of semaphore variable by 1. 也叫做 signal(S)

比如 6P, 2V , 一开始是1 , 那就还有3个进程block, 在等信号量. 

```
wait(){
 S--;
     if(S.value<0 ){
    	从等待队列中加入进程;
    	自我阻塞
    }
}
signal (S) {
	S++;
    if(S.value<=0 ){
    	从等待队列中remove 一个进程P;
    	唤醒P
    }
}
```

一开始S是1, 然后来一个进程S就是0. 再来一个进程,S就小于 0所以就加入等待队列自我阻塞.

Binary semaphore 也叫 mutex locks

同步问题中, 如果action用到资源, 就action前P一下资源. 如果会提供资源, 就行为后V一下资源. 

互斥问题,  P,V要 紧紧夹住使用互斥资源的行为.

#### 生产者和消费者

常考的模型之一

大小为n的缓冲区,同时只能有一个进程访问.  生产和消费是互斥, 又是同步关系.

####  读写问题

 读和写互斥, 写和写互斥, 读和读不互斥.

读要实现和其他读的同步, 同时实现和写互斥,

 常考的模型之一

```c
int count = 0;
semaphore mutex =1;
semaphore rw = 1;
semaphore w =1;
int writer(){
    while(1){
        P(w);
        P(rw);
        writing;
        V(rw);
        V(w);
    }
}
int    reader(){
        while(1){
            P(w);
            P(mutex);
            if (count == 0) {
                P(rw);
            }
            count ++;
            V(mutex);
            V(w);
            reading;
            P(mutex);
            count--;
            if (count == 0) {
                V(rw);
            }
            V(mutex);
        }
    }
```

哲学家问题

方法一:如果拿了左, 检查右边, 如果没有, 放下左. 会导致不停地拿起放下,浪费时间.

最后一题一般都是并发进程同步.  比如用PV原语实现buffer =1 , 三个并发进程的同步. 可以做一做每年的考研题目. 

下面讲几个例题

```c
semopha s = 1;
semopha sa = 0;// 定义1分. 初始1分 
semopha s0 = 0;// 如果只用一个信号量也可以, 但是效率会低. 定义太多了就会死锁. 
main(){
    cobegin
    father();
    son();
    daughter();
    coend
}
father(){
	while(1){
		P(S);
		放入水果
		if (放入橘子)
			V(S0)
		else
			V(Sa)
	}
}
son(){
	while(1){
		P(S0);
		取出橘子;
		V(S);
		吃橘子
	}
}
```

```c
semopha sa = 0;// 定义1分. 初始1分 
semopha s0 = 0;// 如果只用一个信号量也可以, 但是效率会低. 定义太多了就会死锁. 
// 存在两个同步关系
main(){
cobegin
司机();
售货员()
coend
}
driver(){
	while(1){
		wait(S1)// wait售票员 conductor
		启动车
		开车
		停车
		signal(S2)
	}
}
conductor(){
	while(1){
		关车门
		signal(s1)
		售票;
		wait(S2);
	}
}
```

```c
semopha sa = 0;// 定义1分. 初始1分 
semopha s0 = 0;// 如果只用一个信号量也可以, 但是效率会低. 定义太多了就会死锁. 
main(){
cobegin
AB();
BA();
coend
}
AB(){
	while(1){
		wait(s1);
		if (ab == 0) // ab方向没有车
			wait(Sab);//有车就不用等可以直接开
		ab +=1;
		signal(S1);
		a开车到b;
		wait(S1);
		ab -=1;
		if (ab == 0):
			signal (Sab);
		signal(s1);
	}
}
BA(){
	while(1){
		wait(S1);
		开车;
		signal(S);
	}
}
```

打印是不冲突, 可以放在外面

```c
//读写问题和buffer问题的综合问题,很容易写出死锁.
理发师(){
	while(1){
		wait(customer);
		理发;
		signal(customer)
	}
}
customer(){
	while(1){
		wait(mutex);
		if (waiting < 5){
			waiting ++;
			signal(customer);
			signal(mutex);
			wait(理发师);
			wait(mutex);
			waiting --;
			signal(mutex);
		}
		else{
			signal(mutex);
			离开;
		}
	}
}
```

solaris 操作系统 实现了很多同步方法, 但是现在也没人用了. 

linux 提供了spin locks, semaphores. 

write-ahead log

如果不写日志, 数据库会快50%.  不过log每次都是append 操作, io顺序已经是最快.

数据库恢复的时间很长, checkpoint 可以节省recovery time. 

把所有之前的结果output保存到硬盘. 

两阶段锁协议.  一个阶段growing拿所有锁, 第二个释放所有锁. 

timestamp-based 协议 

时间戳也是很难同步的. 

### chapter7 死锁

resource allocation 没有环, 肯定不会死锁.    有环, if only one instance per resource type 就会死锁,  if several instances per resource type, 有可能死锁.

死锁的四个必要条件: 排他 Mutual exclusion ,hold and wait, 不能抢占 No preemption ,循环等待 Circular wait

#### 死锁预防

在设计程序的时候考虑死锁预防

考虑 Hold and Wait – 确保当进程请求资源的时候, 不会占有别的资源

考虑 No preemption -  如果请求一个资源失败, 进程就立刻释放占有的所有资源

考虑 Circular wait - 给所有资源编个号排个序

#### 死锁避免

程序运行时用死锁避免算法动态考虑是否分配. 

每个进程声明它可能需要的每种类型资源的最大数量。
死锁避免算法动态检查资源分配状态，以确保永远不会出现循环等待情况。
资源分配状态由可用和已分配资源的数量以及进程的最大需求定义.当进程请求可用资源时，系统必须决定立即分配是否使系统处于安全状态。

怎么判断?  safe state . 考试会考是否在安全状态, 什么是安全状态

unsafe state 也不一定死锁. 只是可能死锁

##### 避免算法

 Single instance of a resource type. Use a resource-allocation graph. 因为有环就死锁. 时间复杂度是 n平方, 因为图中找环很慢,是 n平方的时间复杂度. 

Multiple instances of a resource type.  用银行家算法

银行家算法

先算出剩余资源和需要的资源,  然后执行序列,   需要的资源<= 剩余的资源才是安全序列

每次执行完一个进程需要加上它已分配的资源.

设Requesti是进程Pi的请求向量，如果Requesti［j］=K，表示进程Pi需要K个Rj类型的资源。当Pi发出资源请求后，系统按下述步骤进行检查：

(1) 若 Requesti[j] ≤ Need[i,j]，转向(2)，否则认为出错（因为它所需的资源数目已超过它所宣布的最大值）。

(2) 若 Requesti[j] ≤ Available[j]，转向(3)，否则须等待（表现为进程Pi受阻）。

(3) 系统尝试把资源分配给进程Pi，并修改下面数据结构中的数值：

> Available[j] = Available[j] – Requesti[j]
> Allocation[i,j] = Allocation[i,j] + Requesti[j]
>  Need[i,j] = Need[i,j] –Requesti[j]

(4) 试分配后，执行安全性算法，检查此次分配后系统是否处于安全状态。若安全，才正式分配；否则，此次试分配作废，进程Pi等待。

 If safe ⇒ the resources are allocated to Pi.
 If unsafe ⇒ Pi must wait, and the old resource-allocation state is restored

死锁检测, 用资源分配图, 

#### 死锁恢复

3种

##### 结束进程

Abort one process at a time until the deadlock cycle is eliminated.

In which order should we choose to abort?  Priority of the process.

##### Resource Preemption

选一个进程结束, 返回safe state, 但是可能导致 starvation.

##### 进程回退

设置还原点回退到回避死锁的地步. 

### chapter8内存管理

#### 分页

 Logical address space of a process can be noncontiguous; process is allocated 物理内存whenever the latter is available
 Divide 物理内存 into fixed-sized blocks called frames (size is power of 2, between 512 bytes and 8,192 bytes) 实际物理内存对应的页就是页框
 Divide logical memory into blocks of same size called pages 

 Keep track of all free frames 

 To run a program of size n pages, need to find n free frames and load program

#### 分配内存方法

 First-fit: Allocate the first hole that is big enough. 最简单, 通常最快最好.  但是容易出现大量小的空闲分区.  增加了查找开销. 
 Best-fit: Allocate the smallest hole that is big enough; must search entire list, unless ordered by size : 好处 Produces the smallest leftover hole  缺点: 产生最多的外部碎片. 
 Worst-fit: Allocate the largest hole; must also search entire list 特点:  Produces the largest leftover hole  性能非常差, 很快就没有可用的内存块了.

First-fit and best-fit better than worst-fit in terms of speed and storage utilization

合并的时候, 算法会空闲分区链 空间从小到大排序.

#### external fragmentation

外部碎片指的是还没有被分配出去（不属于任何进程），但由于太小了无法分配给申请内存空间的新进程的内存空闲区域。外部碎片是除了任何已分配区域或页面外部的空闲存储块。这些存储块的总和可以满足当前申请的长度要求，但是由于它们的地址不连续或其他原因，使得系统无法满足当前申请.


物理地址由MMU计算得到,  逻辑地址由CPU计算得到. 

怎么指向页表?  

Page-table base register (PTBR) points to the page table  

Page-table length register (PTLR) 存储了页表大小

#### TLB

每次要先访问页表, 然后访问物理内存,  访问两次太慢, 所以在 CPU 芯片中，加入了一个专门存放程序最常访问的页表项的 Cache.   a special fast-lookup hardware cache called associative memory or translation look-aside buffers (TLB快表)

可能一次访问, 可能两次访问, 取决于有没有在快表找到. 一般命中率90%以上.  

`Effective Access Time (EAT)  访问时间 =  (访问内存时间+ 访问TLB时间) * hit ratio +( (page table 层数+1)*访问内存时间+访问TLB时间))* (1- hit ratio)`

`Effective Access Time (EAT)  访问时间 =  (EAT without page fault + page fault service time) * page fault rate +( 1- page fault rate)* (EAT without page fault)`

In a demanding paging system, the size of a page is 4KB. A process access the logical address 12345(0x3039)will if the page table is as the following:

0x3039是page3   

进程一开始, page table是空的. 所有bit都是invalid

内存不够了就会page fault , bit 把v改成i, 空间分配给别的进程.

每个进程物理地址不容易分配. 所以给每个进程连续的逻辑空间

#### 三种页表

Structure of the Page Table. 有几十篇的学术论文. 其实很多计算机的课本是论文集
 Hierarchical Paging 多级页表

多级页表, 好处,  如果只有3个地址所在的page 被用到, 只需要 2^4 + 2 *2^4 . 可以节省空间 , 考试会考

保存在内存中的页表承担的职责是将虚拟地址翻译成物理地址。页表一定要覆盖全部虚拟地址空间，不分级的页表就需要有 100 多万个页表项来映射，而二级分页则只需要 1024 个页表项（此时一级页表覆盖到了全部虚拟地址空间，二级页表在需要时创建）。

 Hashed Page Tables 

先hash , 然后找链表, 链表法.  Hash paging 可以和inverted paging一起用 .Hash-based paging sometimes requires more memory accesses因为 hash collision

 Inverted Page Tables

page 太多, 放不进去,而且内存很小的时候, 可以直接给物理内存维护页表. 

好处: 整个os只用维护一个page table 

例题: 

46bit virtual  ,  page size = 8 KB      physical memory = 8GB

a page table entry size = 4bytes. 请问需要几层页表. 

2 ^11的映射关系 

46 - 13 .13是8KB计算得到的page内地址, 33 是高阶地址, 每段11, 需要3层., 一个page table  可以映射2048 pages

#### 分段

对用户友好, 允许维护逻辑上的虚拟空间, 方便人维护 

Segment is good logical unit of information 

• sharing, protection
Page is good physical unit of information 

• simple memory management
Best of both • segmentation on top of paging

分段系统的逻辑地址结构由段号(段名)和段内地址(段内偏移量)所组成。

段号的位数决定了每个进程最多可以分几个段， 段内地址位数决定了每个段的最大长度是多少 

- 多个互相独立的称为**段(segment) 的地址空间**。每个段由一个从0到最大的线性地址序列构成。各个段的长度可以是0到某个允许的最大值之间的任何一个值。不同的段的长度可以不同，并且通常情况下也都不相同。段的长度在运行期间可以动态改变，比如，堆栈段的长度在数据被压入时会增长，在数据被弹出时又会减小。
- 因为每个段都构成了一个独立的地址空间，所以它们可以独立地增长或减小而不会影响到其他的段。
- 要在这种分段或二维的存储器中指示一个地址，程序必须提供两部分地址：**一个段号和一个段内地址**。
- 分段也有助于在几个进程之间共享过程和数据 一个常见的例子就是共享库(shared library)。

- 分段和分页的实现本质上是不同的:页面是定长的而段不是。
- 在系统运行一段时间后内存被划分为许多块，一些块包含着段，一些则成了空闲区，这种现象称为外部碎片**(external fragmentation)。 空闲区的存在使内存被浪费了，而这可以通过**内存紧缩来解决。

先找段表, 再找页表

大page和小page的优缺点是什么? 

小page的缺点:  page table 占用空间大, 

大page的缺点:   大页内存一旦设置，内存就实际分配出去了，也会一直驻留在内存中，不会被交换出去。分配了后有很多intern 碎片.   

#### 页替换策略

Want lowest page-fault rate
 Evaluate algorithm by running it on a particular string of memory references (reference string) and computing the number of page faults on that string
 In all our examples, the reference string is 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5

##### FIFO 

在内存中停留时间最长的. 会考 Belady’s Anomaly: more frames ⇒ more page faults.  LRU 性能较好, 但是需要寄存器和栈的硬件支持, 不会出现belady 异常. FIFO会有belady异常

这个地方很难懂,  frame 是个什么东西? 替换了之后怎么计算虚拟地址? 

##### LRU

置换最近一段时间以来最长时间未访问过的页面

两种近似

LRU Approximation Algorithms

##### Reference bit

  With each page associate a bit, initially = 0  When page is referenced bit set to 1  Replace the one which is 0 (if one exists) 

**Second chance 也叫做NRU not recently use常考**

当一存储块中的页面访问时，其相应的“页面访问”位由硬件自动置“1”，而由页面管理体制软件周期性地（设周期为T，其值通常为几百毫秒），把所有的页面访问位重新置为“0”。这样，在时间T内，某些被访问的页面，其对应的访问位为“1”，而未访问的页面，其对应的访问位为“0”。查寻页面访问位为“0”的页面。在查找过程中，那些被访问的页所对应的访问位被重新置为“0”。由此可见，实际上这种近似LRU算法，已经退化成一种“最近不用”的算法NRU（Not Recently Used）。

 Need reference bit  Clock replacement  If page to be replaced (in clock order) has reference bit = 1 then: 4 set reference bit 0 4 leave page in memory 4 replace next page (in clock order), subject to same rules

##### optimal replacement

Replace page that will not be used for longest period of time

置换以后不再被访问，或者在将来最迟才被访问的页面.  一个进程还未运行完成, 很难估计哪一个页面是以后不再使用或在最长时间以后才会用到的页面。所以该算法是不能实现的。但该算法仍然有意义，作为衡量其他算法优劣的一个标准。

#### page fault

page fault 又分为几种:  major page fault、 minor page fault、 invalid(segment fault)。

major page fault 也称为 hard page fault, 指需要访问的内存不在虚拟地址空间，也不在物理内存中，需要从慢速设备载入。swap 回到物理内存也是 hard page fault。

minor page fault 也称为 soft page fault, 指需要访问的内存不在虚拟地址空间，但是在物理内存中，只需要MMU建立物理内存和虚拟地址空间的映射关系即可。

1. 当一个进程在调用 malloc 获取虚拟空间地址后，首次访问该地址会发生一次soft page fault。
2. 通常是多个进程访问同一个共享内存中的数据，可能某些进程还没有建立起映射关系，所以访问时会出现soft page fault

invalid fault 也称为 segment fault，指进程需要访问的内存地址不在它的虚拟地址空间范围内，属于越界访问，内核会报 segment fault错误。

#### 地址翻译的过程

虚拟地址包括  虚拟页号和页内偏移.  虚拟页号可以分成 TLB tag 和组index.  物理地址分为物理页号和页内偏移.  可以把物理页号等于cache tag, 页内偏移分成 cache 索引和偏移量. 

 先把十六位虚拟地址转  二进制, 找到组index=3 和 TLB tag=3 .查询TLB tag有3, 有效位为1 . 就可以查出来. 

### chapter9 虚拟内存

物理内存不够, 就需要paging disk.  如果内存不足，OS和 SQL Server 会在物理内存和文件缓存之间做大量的Paging 动作（Page的换进和换出

swap 建议内存的2倍大小, 不可以小于一倍大小. 

Process Creation
 Virtual memory 还有其他好处 during process creation: - Copy-on-Write - Memory-Mapped Files (later)

之前讲了用户的内存是怎么分配的. 但是操作系统也需要内存. 

那么, 操作系统是怎么allocate内存的?  请学习buddy system. lab6

windows xp , Processes are assigned working set minimum and working set maximum (50-345 pages) 低于就多分配一些. 

虚拟内存是计算机系统内存管理的一种技术。它使得应用程序认为它拥有连续的可用的内存（一个连续完整的地址空间），而实际上，它通常是被分隔成多个物理内存，还有部分暂时存储在外部磁盘存储器上，在需要时进行数据交换

通过内存地址虚拟化，可以在没有访问某虚拟内存地址时不分配具体的物理内存，在实际访问某虚拟内存地址时操作系统再动态地分配物理内存，建立虚拟内存到物理内存的页映射关系，这种技术称为按需分页（demand paging）

把不经常访问的数据所占的内存空间临时写到硬盘上，这样可以腾出更多的空闲内存空间给经常访问的数据；当CPU访问到不经常访问的数据时，再把这些数据从硬盘读入到内存中，这种技术称为页换入换出（page swap in/out）。这种内存管理技术给了程序员更大的内存“空间”，从而可以让更多的程序在内存中并发运行。

#### MMU

Memory Management Unit是一种负责处理CPU的内存访问请求的计算机硬件。它的功能包括虚拟地址到物理地址的转换（即虚拟内存管理）、内存保护、中央处理器高速缓存的控制。MMU位于处理器内核和连接高速缓存以及物理存储器的总线之间。如果处理器没有MMU，CPU内部执行单元产生的内存地址信号将直接通过地址总线发送到芯片引脚，被内存芯片接收，这就是物理地址。如果MMU存在且启用，CPU执行单元产生的地址信号在发送到内存芯片之前将被MMU截获，这个地址信号称为虚拟地址，MMU会负责把VA翻译成相应的物理地址，然后发到内存芯片地址引脚上。

简而言之，当处理器内核取指令或者存取数据的时候，会提供一个虚拟地址，这个地址是可执行代码在编译的时候由链接器生成的。MMU负责将虚拟地址转换为物理地址，以在物理存储器中访问相应的内容。

### linux 存储管理

Linux操作系统采用了请求式分页虚拟存储 管理方法。
系统为每个进程提供了4GB的虚拟内存空间。 各个进程的虚拟内存彼此独立。Kernel space uses 1G virtual memory, while each of the user processes has its own 3G virtual memory. 共享1GB的内核空间 

每个程序经编译、链接后形成的二进制映像文件有一个代码段和数据段
■ 进程运行时须有独占的堆栈空间

一个进程的用户地址空间主要由mm_struct结构和 vm_area_structs结构来描述。 可能会考
■ mm_struct结构它对进程整个用户空间进行描述（称为内存表述符memory descriptor）
■ vm_area_structs结构对用户空间中各个区间(简称虚存区)进行描述

vm_area_struct结构是虚存空间中一个连续的区域，在这个区域中的信息具有相同的操作和访问特性。
■ 各区间互不重叠,按线性地址的次序链接在一起。 当区间的数目较多时,将建立AVL树以保证搜索速度。AVL 有插入 ,删除就要旋转,  查询比较快, 维护代价高. 红黑树 维护代价小

#### Linux的分页存储管理机制

Linux的页表机制：
■ Linux假定处理器支持三级页表结构。在64位体系结构上采用三级页表机制。对于以IA32体系结构的32位机器,则采用二级页表机制，页中间目录层实际不起作用。
■ 三级分页管理把虚拟地址分成四个位段： 页目录、页中间目录、页表、页内偏移。
■ 系统设置三级页表： ✦ 页目录PGD（Page Directory） ✦ 页中间目录PMD（Page Middle Directory） ✦ 页表PTE（Page Table）
■ 2.6.11以后Linux内核采用四级页表模型来使用硬件分页机制 (支持64位CPU架构) ，分别是：
● Page Global Directory(pgd_t), Page Upper Directory(pud_t),Page Middle Directory(pmd_t) 和Page Table(pte_t)。
● 当硬件分页机制实际是两级页表时，Linux内核把Page Upper Directory 和Page Middle Directory跳过了。

三级分页结构是Linux提供的与硬件无关的分页管理方式。

 ■ 当Linux运行在某种机器上时，需要利用该种机器硬件的存储管理机 制来实现分页存储。
■ Linux内核中对不同的机器配备了不同的分页结构的转换方法。 ■ 对IA32，提供了把三级分页管理转换成两级分页机制的方法。其中一 个重要的方面就是把PGD与PMD合二为一，使所有关于PMD的操作 变为对PGD的操作。

#### 缺页fault

页面异常的处理程序是do_page_fault( )函数， 该函数有两个参数：
Ø一个是指针,指向异常发生时寄存器值存放的地址。
Ø另一个错误码,由三位二进制信息组成： 

ü第0位——访问的物理页帧是否存在； ü 第1位——写错误还是读错误或执行错误； ü 第2位——程序运行在核心态还是用户态。

do_page_fault()函数的执行过程如下：

 Ø 首先得到导致异常发生的线性地址（虚拟地址）,对于 IA32该地址放在CR2寄存器中。
Ø 检查异常是否发生在中断或内核线程中,如是,则进行出 错处理。
Ø 检查该线性地址属于进程的某个vm_area_struct区间。 如果不属于任何一个区间,则需要进一步检查该地址是 否属于栈的合理可扩展区间。一但是用户态产生异常 的线性地址正好位于栈区间的vm_start前面的合理位 置,则调用expand_stack( )函数扩展该区间,通常是扩 充一个页面,但此时还未分配物理页帧。 至此,线性地址必属于某个区间。

Ø根据错误码的值确定下一个步骤: ● 如果错误码的值表示为写错误,则检查该区间是否允 许写,不允许则进行出错处理。
● 如果允许写就是属于写时拷贝(COW)。如果错误码 的值表示为页面不存在,这就是所谓的按需调页 (demand paging)。

写时拷贝的处理过程：
n 首先改写对应页表项的访问标志位，表明其刚被访问过, 这样在页面调度时该页面就不会被优先考虑。
n 如果该页帧目前只为一个进程单独使用,则只需把页表项置为可写。
n 如果该页帧为多个进程共享,则申请一个新的物理页帧并 标记为可写,复制原来物理页帧的内容,更改当前进程相应 的页表项,同时原来的物理页帧的共享计数减一。

按需调页的处理过程:
n 第一种情况页面从未被进程访问,这种情况页表项的值全为0。 

(1)如果所属区间的vm_ops->nopage不为空,表示该区间映射到一个文件,并且vm_ops->nopage指向装入页面的函数,此时调用该 函数装入该页面。
(2)如果vm_ops或vm_ops->nopage为空,则该调用do_anonymous_page( )申请一个页面；
n 另一种情况是该页面被进程访问过,但是目前已被写到交换分区, 页表项的存在标志位为0,但其他位被用来记录该 页面在交换分区中的信息。调用do_swap_page( )函数从 交换分区调入该页面。

Linux使用最近最少使用（LRU）页面置换算法来公平地选择将要从系统中换出的页面。
■ 这种策略为系统中的每个页面设置一个age，它随页面访问次数而变化。页面被访问的次数越多则页面年龄越年轻；相反则越衰老。older页面是待交换页面的最佳侯选者。

在Linux中，进行交换的单位是page而不是进程 

■ 在页面交换中，页面置换算法是影响交换性能的关键性指标，其复杂性主要与换出有关：
● 哪种页面要换出 ● 如何在交换区中存放页面 ● 如何选择被交换出的页面

代码段和内核中的PCB, stack 不会被置换出去.  

#### buddy伙伴算法

 这是lab6
■ Linux把空闲的页帧按照页块大小分组进行管理，数组 free_area[]来管理各个空闲页块组。

伙伴系统是以页帧为基本分配单位, 因而对于小对象容易造成内部碎片。
■ 不同的数据类型用不同的方法分配内存可能提高效率。比如需要初始化的数据结构，释放后可以暂存着，再分配时 就不必初始化了。内核的函数常常重复地使用同一类型的内存区，缓存最近释放的对象可以加速分配和释放。
■ 解决办法：基于伙伴系统的slab分配器。 ■ slab分配器的基本思想：为经常使用的小对象建立缓冲, 小对象的申请与释放都通过slab 分配器来管理。slab 分配 器再与伙伴系统打交道。
■ 好处：其一是充分利用了空间,减小了内部碎片；其二是 管理局部化,尽可能少地与伙伴系统打交道,从而提高了效率。

#### thrash

更换页面时，如果更换页面是一个很快会被再次访问的页面，则再次缺页中断后又很快会发生新的缺页中断。整个系统的效率急剧下降------这种现象称为颠簸thrash（抖动）

----内存颠簸的解决策略是：

1-如果是因为页面替换策略失误，可修改替换算法来解决这个问题 to intersect the running of I/O bound and CPU bound processes

2-如果是因为运行的程序太多，造成程序无法同时将所有频繁访问的页面调入内存，则要减少程序的数量。

3-否则，还剩下两个办法：1终止该进程；2增加物理内存容量；

The working set model  可以计算 minimum (total) number of frames a job will need in order to run smoothly without causing thrashing.

### chapter10 File-System Interface

open file table 

顺序查快很多很多, disk顺序 比random 快100,000倍.   SSD快30,000倍

#### 树结构 

便于文件分类, 但是不利于文件共享. 

#### Acyclic-Graph Directories

硬链接,  另一个用户不能直接删除文件, 只能count-1 然后删除自己目录中的相应目录项. 

软链接, 文件删除后, 通过符号链接访问, 访问 失败, 会删除符号链接. 

#### LFS log结构

那么LFS是怎么减少随机写入呢？它基于一个很简单的 idea: 把整个磁盘看做一个 append only log,永远都是顺序写入。每当我们写入新文件时，我们总是顺序地追加在log 的最后；不同于 FFS/UFS, LFS 对文件的更改并不修改现有内容，而是把增量追加在硬盘的最后。

#### NDFS

问namenode ， 会告诉你去哪里data node

paxos 协议。 

### chapter11 file system implementation

#### Disk structures 

 Boot control block  Volume control block  Directory structure per file system 

 Per-file FCB (inode in UFS, master file table entry in NTFS)

#### In-memory structures 

 In-memory mount table about each mounted volume  Directory cache  System-wide open-file table  Per-process open-file table

Virtual File Systems (VFS) provide an object-oriented way of implementing file systems  java和C调用的都是VFS的文件接口。

怎么找到 FCB？ hash 或者顺序查找。 

找到FCB后， 怎么存数据？  三种方法， 1  contiguous 2 linked  3 indexed 

连续contiguous ， 有时候找不到足够大的空间。 优点是实现简单, 存取速度快.  缺点是文件长度难动态增加. 

#### linked 

Simple – need only starting address  Free-space management system – no waste of space  缺点:  No random access, poor reliability

链表的方式存放是**离散的，不用连续的**，可以**消除磁盘碎片**，可大大提高磁盘空间的利用率，同时**文件的长度可以动态扩展**。

FAT 需要多少块， 多少次 io操作？ 考过一次。 

#### indexed allocation

outer index ->  index table .  mapping

索引的方式优点在于：

- 文件的创建、增大、缩小很方便；Adjustable to any size of files 
- 不会有碎片的问题；
- 支持顺序读写和随机读写；从10th logical block 访问3rd logic block 只用1次disk blocks access.

缺点: Slower random accesses for larger file

 Need index table (analogous to page table)

Combined Scheme: UNIX ext2/3 (4K bytes per block)

Inode 

■ File information ■ The first 12 pointers point directly to data  blocks ■ The 13th pointer points to an index block ■ The 14th pointer points to a block containing  the addresses of index blocks ■ The 15th pointer points to a triple index  block

先渲染前两页, 后面慢慢读.   前面的是 direct blocks, 后面的 io数也比较多.  可能是两层 indirect , 三层 indirect.

#### free space management

空闲表法

bit vector   位图是利用二进制的一位来表示磁盘中一个盘块的使用情况，磁盘上所有的盘块都有一个二进制位与之对应。  这算开创新性的论文

linked free space on disk. 当创建文件需要一块或几块时，就从链头上依次取下一块或几块。反之，当回收空间时，把这些空闲块依次接到链头上。

这种技术只要在主存中保存一个指针，令它指向第一个空闲块。其特点是简单，但不能随机访问，工作效率低，因为每当在链上增加或移动空闲块时需要做很多 I/O 操作，同时数据块的指针消耗了一定的存储空间。空闲表法和空闲链表法都不适合用于大型文件系统，因为这会使空闲表或空闲链表太大。一开始有序， 但是后来就乱序了。 分配速度快。 这算 改进的论文。

grouping的方法，  把连续的几个块group到一起。 这算 普通的论文， 是缝缝补补的论文。 可中可不中。 

linked  

文件系统的要点： 怎么把disk allocation

第二个，怎么快速找文件。 

NFS主要工作是什么？ 

三种磁盘的分配方法。  要熟悉

 熟悉 index ， 熟悉 unix的数据结构。  要理解和认识， 会计算 unix 的最大空间

#### linux文件系统

字符设备文件和块设备文件。Linux把对设备的I/O作为对 文件的读取/写入操作内核提供了对设备处理和对文件处理的统一接口。 

● fd0 (for floppy drive 0) ● hda (for harddisk a) 

● lp0 ( for line printer 0) ● tty(for teletype terminal

管道(PIPE)文件：用于在进程间传递数据。Linux对管道的操作与文件操作相同，它把管道当作文件进行处理。

socket文件

文件系统分三大类： ● 基于磁盘的文件系统，如ext2/ext3/ext4、VFAT、NTFS等。 ● 网络文件系统，如NFS等。 ● 特殊文件系统，如proc文件系统、devfs、sysfs（/sys）等。 

/proc 虚拟文件系统，在这里可以获取系统状态信息并且修改系统的某些配置信息。 ■ 如内存情况在/proc/meminfo文件中，使用命令

■ 支持多种不同类型的文件系统是Linux操作系统的一大特 色。如：ext、ext2、ext3、ext4、minix、iso9660 、 hpfs 、 msdos 、 vfat 、proc 、 nfs 、smb 、 sysv 、ntfs 、ufs、jfs、yaffs 、ReiserFS、 CRAMFS、JFFS2等

Linux在标准内核中已支持的文件系统超过50种。 如果你实现了一个myfilesystem, 也可以认证.

做一个虚拟层,  各种不同物理文件系统转换为一个具有统一 共性的虚拟文件系统。这种转换机制称为虚拟文件系统转 换VFS(Virtual Filesystem Switch/System) 。VFS并不是一种实际的文件系统。ext2/ext4等物理文件系 统是存在于外存空间的，而VFS仅存在于内存。 ■ 在 VFS 上面，是对诸如 open、close、read 和 write 之 类的函数的一个通用 API 抽象。在 VFS 下面是文件系统 抽象，它定义了上层函数的实现方式。文件系统的源代码 可以在 linux/fs 中找到。

很重要的是学习怎么做这种中间件, 因为很多地方都需要这种.

■ Linux的标准文件系统是ext2或ext3或ext4，系统把它的 磁盘分区做为系统的根文件系统。

#### VFS

虚拟内存系统 VFS根据不同的文件系统抽象出了一个通用的文件模型。通用的文件模型由四种数据对象组成： 

● 超级块对象 superblock :存储文件系统的信息 contains all file system configuration parameters

● 索引节点对象 inode object ：存储某个文件的信息。 通常对应磁盘文件系统的文件控制块   fat32 inode实现 一个链表,  ext32是索引结构. 

● 目录项对象dentry object ：dentry对象主要是描述一 个目录项，是路径的组成部分。 ● 文件对象 file object：存储一个打开文件和一个进程的 关联信息。只要文件一直打开，这个对象就一直在内存。

```c
struct super_block {
struct list_head s_inodes;// inodes 很多, 都在磁盘里. 
struct list_head s_dirty;
}
```

在系统运行中， VFS要建立、撤消一些VFS inode，还要对VFS超级块进行一些必要的操作。这些操作由一系列操作函数实现。 ■ 不同类型的文件系统的组织和结构不同，完成同样功能的操作函数的 代码不同，每种文件系统都有自己的操作函数。  如何在对某文件系统进行操作时就能调用该文件系统的操作函数呢？ 这是由VFS接口通过转换实现的。 ■ 在VFS超级块中s_op是一个指向super_operations结构的指针， super_operations中包含着一系列的操作函数指针，即这些操作函 数的入口地址。 

■ 每种文件系统VFS超级块指向的super_operations中记载的是该文件 系统的操作函数的入口地址。只需使用它们各自的超级块成员项s_op， 以统一的函数调用形式：s_op->read_inode()就可以分别调用它们各自的读inode操作函数

产生 super block , 修改superblock , 对superblock 支持的inode进行操作.

```c
struct inode { 
nlink_t i_nlink; /* 该文件的链接数 */
uid_t i_uid; /* 文件所有者的用户标识 */
gid_t i_gid; /* 文件的用户组标识 */
time_t i_atime; /* 文件最后一次访问时间 */
time_t i_mtime; /* 文件最后一次修改时间 */
time_t i_ctime; /* 文件创建时间 */
unsigned long i_blksize; /* 块尺寸，以字节为单位 */
unsigned long i_blocks; /* 文件的块数 */
struct inode_operations *i_op; /* 指向inode操作函数入口表的指针 */
    
}
```

目录项对象dentry object ■ 每个文件除了有一个索引节点inode数据结构外，还有一个 目录项dentry数据结构。 ■ 每个dentry代表路径中的一个特定部分。如：/、bin、vi都 属于目录项对象。 ■ 目录项也可包括安装点，如：/mnt/cdrom/foo，/、mnt、 cdrom、foo都属于目录项对象。 ■ 目录项对象作用是帮助实现文件的快速定位，还起到缓冲作用

练习题:

Which one of below is a block device?

块设备是i/o设备中的一类，是将信息存储在固定大小的块中，每个块都有自己的地址，还可以在设备的任意位置读取一定长度的数据，例如硬盘,U盘，SD卡等。

1.SCSI硬盘 —— 以sd开头。 例如：sda表示第一块SCSI硬盘、sdb表示第二块SCSI硬盘、sda1表示第一块SCSI硬盘的第一个分区。 2.IDE硬盘 —— 以hd开头。

EXT3是**第三代扩展文件系统**（ 英语 ：Third extended filesystem，缩写为ext3），是一个日志文件系统，常用于 Linux操作系统 。             

4GB ,  1024个indirect blocks,   1个double -indirect  block. 256 bytes for inode。

A file system uses a two-level indexed allocation scheme. If the size of each block is 1KB, and the block address is 4 bytes, then the maximum length of a file this system can support is approximately                                     

256 * 256*1KB

```
#define NR_OPEN 256
struct files_struct {
int count; /* 共享该结构的计数值 */
fd_set close_on_exec; 
fd_set open_fds;
struct file * fd[NR_OPEN]; 
};
```

#### 进程的文件管理

 ● 对于一个进程打开的所有文件，由进程的两个私有结构进行管理。 fs_struct结构记录着文件系统根目录root和pwd当前目录，  files_struct结构包含着进程的打开文件表

 fd[]每个元素是一个指向file结构体的指针，该数组称为进程打开文件表。 

■ 进程打开一个文件时，建立一个file结构体，并加入到系 统打开文件表中，然后把该file结构体的首地址写入fd[]数 组的第一个空闲元素中，一个进程所有打开的文件都记载 在fd[]数组中。

■ fd[]数组的下标称为**文件描述符** file description。 ■ 在Linux中，进程使用文件名打开一个文件。 在此之后对 文件的识别就不再使用文件名，而直接使用文件描述符。 ■ 在系统启动时文件描述符0、1、2由系统分配: 0标准输入 设备，1标准输出设备，2标准错误输出设备。 ■ 当一个进程通过fork()创建一个子进程后，子进程共享父进程的打开文件表，父子进程的打开文件表中下标相同的 两个元素指向同一个file结构。 这时file的f_count计数值增1。 ■ 一个文件可以被某个进程多次打开，每次都分配一个file 结构，并占用该进程打开文件表fd[]的一项，得到一个文 件描述符。但它们的file结构中的f_inode都指向同一个 inode

ext2的绝大多数的数据结构和系统调用与经典的UNIX一致。 ■ 能够管理海量存储介质。支持多达4TB的数据，即一个分区的容量 最大可达4TB。 ■ 支持长文件名，最多可达255个字符，并且可扩展到1012个字符 ■ 允许通过文件属性改变内核的行为；目录下的文件继承目录的属性。 ■ 支持文件系统数据“即时同步”特性，即内存中的数据一旦改变， 立即更新硬盘上的数据使之一致。 ■ 实现了“符号连接”（symbolic links）的方式，使得连接文件只 需要存放inode的空间。 ■ 允许用户定制文件系统的数据单元（block）的大小，可以是 1024、 2048 或 4096 个字节，使之适应不同环境的要求。 ■ 使用专用文件记录文件系统的状态和错误信息，供下一次系统启动 时决定是否需要检查文件系统 

