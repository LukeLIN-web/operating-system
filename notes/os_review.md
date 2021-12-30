- 40选择 1.5一个 10分填空 30分大题

- DMA：不在CPU参与的情况下在设备之间搬运数据

- 注意措辞和RISCV的区别

  - Interrupts and Traps
  - 确定中断源：polling / vectored interrupt system

- SMP：共享物理memory，Symmetric Multiprocessing Architecture

- NUMA：Non-Uniform Memory Access

  <img src="os_review.assets/image-20210111141833960.png" alt="image-20210111141833960" style="zoom: 15%;" />

- 等待信号量：running状态到blocked状态

- 选择题比较多

- 进程间通信 IPC

  <img src="os_review.assets/image-20210111144301207.png" alt="image-20210111144301207" style="zoom: 50%;" />

  - 共享内存
  - 消息传递

- 同步肯定有一道大题

- 同步

  - Race condition
  - CS Critical Section



- monolithic

- batch / time sharing / real time

- B（主存会涉及到，因为需要读写PCB）

  ![image-20210114221907161](os_review.assets/image-20210114221907161.png)



### JJM习题

- ![image-20210115102456970](os_review.assets/image-20210115102456970.png)

- <img src="os_review.assets/image-20210115103412475.png" alt="image-20210115103412475" style="zoom: 67%;" />

- Solution to Critical-Section: Three Requirements

- working set

- <img src="os_review.assets/image-20210115104050285.png" alt="image-20210115104050285" style="zoom:50%;" />

  <img src="os_review.assets/image-20210115104217063.png" alt="image-20210115104217063" style="zoom:50%;" />

- <img src="os_review.assets/image-20210115130859407.png" alt="image-20210115130859407" style="zoom:50%;" />

- ![image-20210115132733258](os_review.assets/image-20210115132733258.png)

- thrashing

  - the processor spends most of its time in swapping pages, rather than executing them

- Belady’s Anomaly 异常

  - 采用FIFO算法时，如果对—个进程未分配它所要求的全部页面，有时就会出现分配的页面数增多但缺页率反而提高的异常现象

- 页替换的second chance

  - 指针指向的是下一个要被替换的page

  <img src="os_review.assets/image-20210115135202942.png" alt="image-20210115135202942" style="zoom:50%;" />

- ![image-20210115135517954](os_review.assets/image-20210115135517954.png)

- Name three ways in which the processor can transition form user mode to kernel mode? 

  - 1) The user process can execute a trap instruction (e.g. system call).A trap is known asa synchronous software interrupt.

    2) The user process can cause an exception (divide by zero, access badaddress, bad instruction, page fault, etc).

    3) The processor can transition into kernel mode when receiving aninterrupt.

- <img src="os_review.assets/image-20210115142623679.png" alt="image-20210115142623679" style="zoom: 50%;" />

- <img src="os_review.assets/image-20210115161030197.png" alt="image-20210115161030197" style="zoom:50%;" />

- context switch

  - Ingeneral, the operating system must **save the state** of the currently running process and restore the state of the process scheduled to be run next. Saving the stateof a process typically includes the values of all the CPU registers in additionto memory allocation. 
  - **Context switches must also perform manyarchitecture-specific operations, including flushing data and instruction caches.**

- <img src="os_review.assets/image-20210115163506088.png" alt="image-20210115163506088" style="zoom:50%;" />

- <img src="os_review.assets/image-20210115163537082.png" alt="image-20210115163537082" style="zoom:50%;" />

- ![image-20210115163758034](os_review.assets/image-20210115163758034.png)

- ![image-20210115164253919](os_review.assets/image-20210115164253919.png)

- <img src="os_review.assets/image-20210115164513777.png" alt="image-20210115164513777" style="zoom:50%;" />

- 响应比=作业响应时间/作业执行时间=(作业执行时间十作业等待时间)/作业执行时间。高响应比调度算法在等待时间相同的情况下，作业执行时间越短响应比越高，满足短任务优先。随着等待时间增加，响应比也会变大，执行机会就增大，所以不会产生饥饿现象。先来先服务和时间片轮转不符合短作业优先，非抢占式短作业优先会产生饥饿现象。

  <img src="os_review.assets/image-20210115170931432.png" alt="image-20210115170931432" style="zoom: 67%;" />

  <img src="os_review.assets/image-20210115165031330.png" alt="image-20210115165031330" style="zoom:50%;" />

- 调度相关术语

  - throughput: the number of processes completed per time unit

  <img src="os_review.assets/image-20210115170108946.png" alt="image-20210115170108946" style="zoom:50%;" />

- ![image-20210117233847426](os_review.assets/image-20210117233847426.png)

- ![image-20210117234048070](os_review.assets/image-20210117234048070.png)

- <img src="os_review.assets/image-20210118101346933.png" alt="image-20210118101346933" style="zoom:50%;" />

- inverted page table

  ![image-20210118104236776](os_review.assets/image-20210118104236776.png)

- ![image-20210118104931670](os_review.assets/image-20210118104931670.png)

- EAT

- <img src="os_review.assets/image-20210118154234667.png" alt="image-20210118154234667" style="zoom: 67%;" />

  ![image-20210118154046072](os_review.assets/image-20210118154046072.png)

- ![image-20210118154350787](os_review.assets/image-20210118154350787.png)

- second-chance？

  ![image-20210118154735499](os_review.assets/image-20210118154735499.png)

- NRU？

  ![image-20210118154935386](os_review.assets/image-20210118154935386.png)

- <img src="os_review.assets/image-20210118155010737.png" alt="image-20210118155010737" style="zoom:50%;" />

- 信号量相关习题

  - https://www.gatevidyalay.com/semaphore-binary-semaphore-practice-problems/

- <img src="os_review.assets/image-20210119143130528.png" alt="image-20210119143130528" style="zoom:50%;" />

- <img src="os_review.assets/image-20210119143612610.png" alt="image-20210119143612610" style="zoom: 50%;" />



### Introduction

- ssd，均匀擦除 solid state disk
- 系统调用传递参数：寄存器，内存的block，压栈



