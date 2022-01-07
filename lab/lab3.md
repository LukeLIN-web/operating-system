# Lab 3: RISC-V64进程调度模拟 

## 1 实验目的

结合课堂所学习的相关内容，在上一实验实现时钟中断的基础上进一步实现简单的进程调度。

## 2 实验内容及要求

* 理解进程调度与进程切换过程
* 利用时钟中断模拟进程调度实验，实现优先级抢占式算法和短作业优先非抢占式算法

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，跟随实验步骤完成实验，以截图的方式记录命令行的输入与输出，注意进行适当的代码注释。如有需要，请对每一步的命令以及结果进行必要的解释。

最终提交的实验报告请命名为“**学号1_姓名1\_学号2\_姓名2\_lab3.pdf**"，代码文件请根据给出的结构整理好后打包命名为“**学号1_姓名1\_学号2\_姓名2\_lab3**"，分开上传至学在浙大平台。

本实验以双人组队的方式进行，**仅需一人提交实验，**默认平均分配两人的得分（若原始打分为X，则可分配分数为2X，平均分配后每人得到X分）。如果有特殊情况请单独向助教反应，出示两人对于分数的钉钉聊天记录截图。单人完成实验者的得分为原始打分。

## 3 实验步骤

### 3.1 环境搭建

#### 3.1.1 建立映射

同lab2的文件夹映射方法，目录名为lab3。

```shell
docker run -it -v /c/Users/12638/Desktop/operatingSystem/lab/lab3:/home/oslab/lab3 -u oslab -w /home/oslab fa20 /bin/bash
 docker exec -it  -u oslab -w /home/oslab determined_davinci /bin/bash
```

#### 3.1.2 组织文件结构

```
lab3
├── arch
│   └── riscv
│       ├── kernel
│       │   ├── clock.c
│       │   ├── entry.S
│       │   ├── head.S
│       │   ├── init.c
│       │   ├── main.c
│       │   ├── Makefile
│       │   ├── print.c
│       │   ├── sbi.c
│       │   ├── sched.c
│       │   ├── test.c
│       │   ├── trap.c
│       │   └── vmlinux.lds
│       └── Makefile
├── include
│   ├── defs.h
│   ├── riscv.h
│   ├── sched.h
│   ├── stddef.h
│   ├── stdio.h
│   └── test.h
└── Makefile
```

**首先请阅读【附录A.进程】确保对实验相关知识有基本的了解。**

本实验意在模拟操作系统中的进程调度，实验中将定义结构体模拟进程的资源，利用时钟中断模拟CPU时间片以触发调度算法。 程序执行时，首先将`task[1-4]`的剩余运行时间设为0，通过时钟中断处理函数引起第一次调度，`task0`使用一个时间片运行`init_test_case()`函数，根据`counter_priority`数组为进程分配相应的时间片与优先级，随后进行进程调度，根据运行结果观察调度算法实现正确性。

代码的总体框架工程文件均已给出，需要修改的部分均已进行了标注，主要包括：

* `entry.s`：
  * `__init_epc`：将`sepc`寄存器置为`test()`函数。
  * `trap_s`：首先保存当前的寄存器，随后调用`handle_s`函数，最后恢复寄存器。
  * `__swtich_to`：需要保存当前进程的执行上下文，再将下一个执行进程的上下文载入到相关寄存器中。
* `sched.c`：
  * `task_init()`：对`task0`、`task[1-4]`进行初始化，设置`current`变量指示当前执行的进程。
  * `do_timer()`： 在时钟中断处理中被调用，首先会将当前进程的剩余运行时间减少一个单位，之后根据调度算法来确定是继续运行还是调度其他进程来执行。 一秒运行一次do timer
  * `schedule()`：根据调度算法，考虑所有可运行的进程的优先级和剩余运行时间，按照一定规则选出下一个执行的进程。如果与当前进程不一致，则需要进行进程切换操作。如果当前进程均已执行完毕，为`task0`新增一个时间片以重新分配优先级和运行时间。
  * `switch_to`：进程切换，更新`current`变量，调用`__switch_to`函数切换进程的执行上下文。

此外，代码中有部分内容涉及lab2、lab1的实现，没有给出具体实现，需要同学们根据之前的代码自行替换，注意文件内容有部分更新，请不要直接替换文件：`init.c`的`init()`函数中有新增`task_init()`的调用；`trap.c`中有更改，中断处理函数中调用了`do_timer()`模拟CPU时间片运行；`clock.c`中`timebase`设为`100000`以快速获得实验结果。

### 3.2 sched.c进程调度功能实现（60%）

#### 3.2.1 调度算法切换

本实验需要实现两种调度算法，可以通过宏定义及编译选项`gcc –D`进行控制。

* 修改`lab3/Makefile`中的 `CFLAG = ${CF} ${INCLUDE} -DSJF / -DPRIORITY` 以控制实验所采用的调度算法。
  * -DSJF （短作业优先式）。
  * -DPRIORITY （优先级抢占式）。
* 在`sched.c`中使用`#ifdef`,`#endif`语句来控制进程调度的代码实现。

#### 3.2.2 实现 task_init()（10%）

本实验中，我们使用了多个进程，需要对物理内存区域进行划分。此次实验中我们手动做内存分配，把物理地址空间划分成多个帧(frame)。即，从`0x80010000`地址开始，连续地给此次实验的 4 个 Task [1-4] 以及内核栈Task[0]做内存分配，我们以 `4KB` 为粒度，按照每个 Task 一帧的形式进行分配，并将`task_struct` 存放在该帧的低地址部分， 将栈指针 `sp` 指向该帧的高地址。**（请同学按照下图的内存空间分配地址，不要随意修改，否则有可能影响到最终的实验结果）**

```shell
---------------------------------------------------------------------------
|             |   Task0   |   Task1   |   Task2   |   Task3   |   Task4   |
|   Kernel    |           |           |           |           |           |
|             |   Space   |   Space   |   Space   |   Space   |   Space   |
---------------------------------------------------------------------------
^             ^           ^           ^           ^           ^
0x80200000    0x80210000  0x80211000  0x80212000  0x80213000  0x80214000
```

可将Task [1-4] 看作是从 0 号进程(Task [0] ) fork 出的子进程，后续实验的实现将主要考虑如何对这四个子进程进行调度。为方便起见，我们将Task [1-4] 进程均设为`dead_loop()`（见`test.c`）

在`task_init()`函数中对实验中的进程进行初始化设置：

* 初始化`current`与`task[0]`
  * 设置`current`指向`Task0 Space`的基地址。
* 设置`task[0]` 为 `current`。
  * 初始化`task[0]`中的成员变量：`state` = TASK_RUNNING；`counter` = 1；`priority` = 5；`blocked` = 0；`pid` = 0。
  * 设置`task[0]`的`thread`中的`sp`指针为 Task0 Space基地址 + 4KB的偏移。
  
* 参照`task[0]`的设置，对`task[1-4]`的成员变量完成初始化设置 
  * `counter` =  1
  * `priority` = 5

* 在初始化时，我们需要将`thread`中的`ra`指向一个初始化函数`__init_sepc`(entry.S)，在该函数中我们将`test`函数地址赋值给`sepc`寄存器，详见【3.3.1】

```c
void task_init(void) {
    puts("task init...\n");

    //initialize task[0]
    current = (struct task_struct*)Kernel_Page;
    current->state = TASK_RUNNING;
    current->counter = 1;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;
    task[0] = current;
    task[0]->thread.sp = (unsigned long long) task[0] + TASK_SIZE; //设置`current`指向`Task0 Space`的基地址。

    //set other 4 tasks
    for (int i = 1; i <= LAB_TEST_NUM; ++i) {
        /*your code*/
        struct task_struct* tmp = (struct task_struct*)(Kernel_Page+PAGE_SIZE*i);
        tmp->state = TASK_RUNNING;
        tmp->counter = 0;
        tmp->priority = 5;
        tmp->blocked = 0;
        tmp->pid = i;
        task[i] = tmp;
        task[i]->thread.sp = (unsigned long long) task[i] + TASK_SIZE; 
        task[i]->thread.ra = &__init_sepc;
        printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
    }
    task_init_done = 1;
}

```

#### 3.2.3 短作业优先非抢占式算法实现（20%）

* `do_timer()`
  * 将当前所运行进程的剩余运行时间减少一个单位（`counter--`）
  * 如果当前进程剩余运行时间已经用完，则进行调度，选择新的进程来运行，否则继续执行当前进程。
* `schedule()`
  * 遍历进程指针数组`task`，从`LAST_TASK`至`FIRST_TASK`(不包括`FIRST_TASK`，即task[0])，在所有运行状态(TASK_RUNNING)下的进程剩余运行时间最小的进程作为下一个执行的进程。若剩余运行时间相同，则按照遍历的顺序优先选择。
  * 如果所有运行状态下的进程剩余运行时间都为0，给`task 0`分配一个时间片，由其通过`init_test_case()`函数为其余进程分配剩余运行时间与优先级，重新开始调度。

```c
#ifdef SJF 
//simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void) {
    if (!task_init_done) return;
    if (task_test_done) return;
    printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter,current->priority);
    //current process's counter -1, judge whether to schedule or go on.
    /*your code*/
    current->counter --;
    if(current->counter == 0)
        schedule();
}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    int next = -1; 
    long  min = 999;
    for(int i = LAB_TEST_NUM ;i >= 1;i --){
        if(task[i]->counter == 0)
            continue;
        if(task[i]->counter < min){
            next = i;
            min = task[next]->counter;
        }
    }
    if(next == -1){
        next = 0;
        task[0]->counter = 1;
    }
    if(next != -1 && current->pid != task[next]->pid){
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n", 
        current->pid,task[next]->pid,
        current->pid, (unsigned long)current->thread.sp, 
        task[next]->pid, (unsigned long)task[next], 
        task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif
```

#### 3.2.4 优先级抢占式算法实现（20%）

* `do_timer()`
  
  * 将当前所运行进程的剩余运行时间减少一个单位（`counter--`）
  * 每次`do_timer()`都进行一次抢占式优先级调度。
  
* `schedule()`
  
  * 遍历进程指针数组`task`，从`LAST_TASK`至`FIRST_TASK`(不包括`FIRST_TASK`)，调度规则如下：
    * 高优先级的进程，优先被运行。
    
    * 若优先级相同，则选择剩余运行时间少的进程（若剩余运行时间也相同，则按照遍历的顺序优先选择） 
    
       少但是不能为0 
    
  * 如果所有运行状态下的进程剩余运行时间都为0，给`task 0`分配一个时间片，通过`init_test_case()`函数为其余进程分配剩余运行时间与优先级，重新开始调度。

```c
#ifdef PRIORITY

//simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void) {
    if (!task_init_done) return;
    if (task_test_done) return;
    printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter,current->priority);
    //current process's counter -1, judge whether to schedule or go on.
    /*your code*/
    current->counter --;
    if(current->counter == 0)
        schedule();
}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    unsigned char next = -1; 
    /*your code*/
    int maxPriority = -1;
    int remainShortest = 999;
    char flag = 2 ;
    for (int i = LAB_TEST_NUM; i >= 1; i--){
        /* code */
        if(task[i]->counter <= 0)
            continue;
        if( task[i]->priority > maxPriority ){
            next = i ; 
            maxPriority = task[i]->priority;
            // remainShortest = task[i]->counter;
        }
        else{
            if ( task[i]->priority == maxPriority){
                if( task[i]->counter < remainShortest ){
                    next = i ;// if they have equal priority, don't change 'next' since we select by iteration order.
                    remainShortest = task[i]->counter;
                }
            }
        }
        if (task[i]->counter > 0)
            flag = 1;
    }
    if (flag == 2  ){
        // allocate a time slice to task0
        task[0]->counter = 1;
        //init_test_case();// I wonder whether we can call it directly, we do not declare it ?
        next = 0;
    }
    if(current->pid != task[next]->pid){
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n", 
        current->pid,task[next]->pid,
        current->pid, (unsigned long)current->thread.sp, 
        task[next]->pid, (unsigned long)task[next], 
        task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif
```

#### 3.2.5 实现 switch_to(struct task_struct* next)（10%）

* 判断下一个执行的进程`next`与当前的进程`current`是否为同一个进程，如果是同一个进程，则无需做任何处理。如果不一致，则更新`current`变量，并调用`__switch_to(struct task_struct* prev,struct task_struct* next)`(`entry.S`)。

  ```c
  #sched.c
  //If next==current,do nothing; else update current and call __switch_to.
  void switch_to(struct task_struct* next) {
      /*your code*/
      if(next->pid != current->pid){
          struct task_struct* prev = current;
          current = next;
          __switch_to(prev,current);
      }
  }
  ```

### 3.3 实现entry.s（20%）

#### 3.3.1 __init_epc（10%）

```asm
.globl __init_sepc
__init_sepc:
	la  t0, test
	csrw sepc, t0
	sret
```

原因在于，运行中的进程遇到时钟中断后，需要将当前运行线程的上下文环境保存在栈上 ( `entry.S`中的`trap_s` 函数)，当线程再次被调度时，再将上下文从栈上恢复。但是对于新创建的、初次被进行调度的任务 ，其栈上还没有可以被`trap_s`函数用来恢复上下文的寄存器信息，所以需要为其第一次调度提供一个特殊的返回函数，将其寄存器信息填写到栈上。

具体的方法是，将其任务结构体中的`ra`寄存器填写为`__init_epc`的地址，在`sret`时返回`test()`函数作为该任务的代码段，此时由于还未对时钟中断进行处理，会重新回到`trap_s`，新任务的寄存器信息便被填到了栈上。

Q1：使用gdb调试观察，说明第一个测试用例中，`task1`切换到`task2`，`task2`初次被调度时的每个函数的运行内容及函数间的跳转过程【自行补充完成】

答：

* task1执行中，时钟中断触发
* trap_s:   将当前运行线程的上下文环境保存在栈上
* handler_s:  如果设置的时间到了, 那就do_timer() 把counter -1.
* do_timer: 打印目前counter和priority counter -1 , 如果为0 就schedule
* schedule:  选择下一个线程 
* switch_to->__switch:  跳转到线程2,  保存上下文, 取出线程2的上下文装入寄存器 
* __init_epc: 将sepc设置为test函数的地址，sret指令使得pc被填充为sepc的值，即进入test函数。
* trap_s：时钟中断未处理，再次进入trap_s保存上下文
* handler_s: 定时触发dotimer
* trap_s：恢复上下文
* task2执行

#### 3.3.2 在trap_s中断处理中添加保存epc的指令

当异常发生时，`epc`寄存器会保存当前的异常返回地址，该地址是与当前task相关的。由于task会在异常处理中发生调度，所以我们需要在中断处理时保存当前的`epc`，当中断处理完成之后再载入`epc`。本实验发生在S模式，因此使用的是`sepc`寄存器，实现同`lab2`。

#### 3.3.3  __switch_to(struct task_struct\* prev, struct task_struct\* next)（10%）

实现切换进程的过程，`a0`参数为`struct task_struct* prev`，即切换前进程的地址。

* 保存当前的`ra`, `sp`, `s0~s11`寄存器到当前进程的`thread_struct`结构体中。
* 将下一个进程的`thread_struct`的数据载入到`ra`, `sp`, `s0~s11`中。

```asm
#entry.S
__switch_to:
li a4, 40
add a3, a0, a4
add a4, a1, a4
#Save context into prev->thread
/*your code*/
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
#Restore context from next->thread
/*your code*/
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
#return to ra
ret
```

**Q2：为什么要设置前三行汇编指令？**

答：a0是当前结构的开始, a1是下一个进程的地址.  a3和a4分别是当前的和下一个进程的进程状态段数据结构地址, 40bytes 是struct 前面部分 size的大小. 结构体task_struct的成员变量的总大小为40 a0为当前结构体的开始，向后偏移40后来到 thread_struct thread的地址处，并将其地址保存到a3中。 同理将下一个thread的地址保存到a4，便于 后面的sd和ld

### 3.5 编译及测试（20%）

#### 1. 运行截图（10%）

本实验按照默认设置的第一个测试用例的预期实验结果如下，截图中必须截到打印的第一行内容，即本组成员的学号与姓名。

##### 1 短作业优先非抢占式 算法

##### 2 优先级抢占式算法 

按照优先级 4321

![image-20211121234555006](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211121234555006.png)

按照时间 短4321

![image-20211121234603197](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211121234603197.png)

按照遍历顺序, 4321

![image-20211121234631923](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211121234631923.png)



#### 2.测试用例（10%）

实验验收时将修改`test.c`中的`counter_priority`数组改变进程的优先级及剩余运行时间，确保代码在不同运行情况下的正确性。同学们自行设置测试用例进行实验以确保代码在边界情况的准确性，将测试用例按照格式补充在表格中。

##### 1 短作业优先非抢占式算法

| 测试目标                         | 测试用例(counter_priority) | 预期结果 | 实际结果 |
| -------------------------------- | -------------------------- | -------- | -------- |
| 随机1                            | {1,4},{2,5},{3,2},{4,1}    | 4>3>2>1  |          |
| 随机2                            | {5,1},{3,3},{4,1},{2,1}    | 1>3>2>4  |          |
| 相同剩余运行时间，以遍历顺序执行 | {1,1},{1,1},{1,1},{1,1}    | 4>3>2>1  |          |
|                                  | {1,1},{1,2},{1,3},{1,4}    |          | 4>3>2>1  |

##### 2 优先级抢占式算法

| 测试情况                             | 测试用例(counter_priority) | 预期结果 | 实际结果 |
| ------------------------------------ | -------------------------- | -------- | -------- |
| 不同优先级                           | {1,1},{1,2},{1,3},{1,4}    | 4>3>2>1  | 4>3>2>1  |
| 相同优先级, 不同剩余时间             | {4,1},{3,1},{2,1},{1,1}    | 4>3>2>1  | 4>3>2>1  |
| 相同优先级和剩余时间，以遍历顺序执行 | {1,1},{1,1},{1,1},{1,1}    | 4>3>2>1  | 4>3>2>1  |
| 相同优先级, 不同剩余时间             | {1,4},{2,5},{3,2},{4,1}    | 2 1 3 4  | 2 1 3 4  |

## 4 讨论和心得

请在此处填写实验过程中遇到的问题及相应的解决方式。

由于本实验为新实验，可能存在不足之处，欢迎同学们对本实验提出建议。

#### 一开始是pid 4不是0

debug

```
riscv64-unknown-linux-gnu-gdb lab3/vmlinux
target remote localhost:1234
```

因为我没有复制clock代码. 

复制了之后还是没有显示.

在trap_s: 的最后 sret,再step就卡住了.

为什么一开始current是4呢.

先init , 然后test. 我不应该用current来初始化1-4 , 我改成了tmp

#### 没有switch

不能switch到pid1:  因为task[63]是空指针, 所以没有-> counter.   *long* c = task[LAB_TEST_NUM]->counter;

不停地allocate , running task0 , 但是task1的counter没有减少. 

因为 current = next  然后 switch to(current ,next) 就不会. 应该先switch  然后改变 current.

#### 跳到0不切换.

`  task[i]->thread.ra = __init_sepc;`  可能有问题, 要取地址. 

0x1L 就是long 型整数.

{1,1},{3,1},{2,1},{4,1}  这里,  current->pid ==  task[next]->pid  都是1.

所以就是要判断> 0 

#### 跑完三个卡住了

他没有按照优先级来, 

没有设置.

##### 无法重新分配

unsigned char next =-1 不会报error, 但是运行会卡住.

char 也不行

signed char 和char不一样.  C标准中对char是 Impementation Defined，就是未明确定义 （1）那它由什么定义？坦白说，具体的编译器明确定义，一般都是用signed char或unsigned char来实现char的，也就是说不同的编译器对char是什么定义不一样 （2）为什么要这样定义？因为这三种类型的对象在存储介质中的表现形式是一样的（都是一个占8bit的01串，只是解析的时候不同） （3）到底是signed char还是unsigned char？这得看编译器：VC编译器、x86上的GCC都把char定义为signed char，而arm-linux-gcc却把char定义为 unsi…

## 附录

### A.进程调度

#### 1. 进程与线程

源代码经编译器一系列处理（编译、链接、优化等）后得到的可执行文件，我们称之为程序（Program）。而通俗地说，进程（Process）就是**正在运行**并使用计算机资源的程序。为了运行程序，操作系统需要给进程分配一定的资源——程序的代码、数据段被加载到内存中，构建程序所需的虚拟内存空间，以及分配其他页表、文件等资源。

出于OS对计算机系统精细管理的目的，线程的概念将“正在运行”的动态特性从进程中剥离出来，这样的一个借助 CPU 和栈的执行流，我们称之为**线程 (Thread)** 。因此，进程虽然仍是代表一个正在运行的程序，但是其主要功能是作为**资源的分配单位**，管理页表、文件、网络等资源。而一个进程的多个线程则共享这些资源，专注于执行，从而作为**执行的调度单位**。

举一个例子，为了分配给进程一段内存，操作系统把一整个页表交给进程，此时进程内部的所有线程看到的是同样的页表，即相同的地址，但是不同的线程有自己的栈（会放在相同地址空间的不同位置），CPU 也会以线程作为一个基本调度单位.一个进程可以有多个线程，也可以如传统进程一样只有一个线程。在本次调度算法模拟实验中，我们**假设**操作系统未引入线程的概念，即一个进程只有一个线程，因此对线程的调度可以简化为对进程的调度。但同学们还是要记住，在引入线程的操作系统中，**进程是资源分配的最小单位，而线程是调度的最小单位**。

#### 2. 本实验中进程调度与切换的过程

* 在每次**时钟中断处理**时，操作系统首先会将当前进程的剩余运行时间减少一个单位。之后根据调度算法来确定是继续运行还是调度其他进程来执行。
* 在**进程调度**时，操作系统会对所有可运行的进程进行判断，按照一定规则选出下一个执行的进程。如果没有符合条件的进程，则会对所有进程的优先级和剩余运行时间相关属性进行更新，再重新选择。最终将选择得到的进程与当前进程切换。
* 在**进程切换**的过程中，首先我们需要保存当前进程的执行上下文，把将要执行进程的上下文载入到相关寄存器中，至此我们完成了进程的调度与切换。

#### 3. 进程的表示

在不同的操作系统中，为每个进程所保存的信息都不同。在这里，我们提供一种**基础的实现**，每个进程会包括：

* 进程ID：用于唯一确认一个进程。
* 运行时间片：为每个进程分配的运行时间。
* 优先级：在调度时，配合调度算法，来选出下一个执行的进程。
* 运行栈：每个进程都必须有一个独立的运行栈，保存运行时的数据。
* 执行上下文：当进程不在执行状态时，我们需要保存其上下文（其实就是状态寄存器的值），之后才能将其恢复，继续运行。

相应的，`sched.h`中数据结构定义如下，请确保理解后再进行之后的实验。

```C
#ifndef _SCHED_H
#define _SCHED_H

#define TASK_SIZE   (4096)
#define THREAD_OFFSET  (5 * 0x08)

#ifndef __ASSEMBLER__

/* task的最大数量 */
#define NR_TASKS    64

#define FIRST_TASK  (task[0])
#define LAST_TASK   (task[NR_TASKS-1])

/* 定义task的状态，Lab3中task只需要一种状态。*/
#define TASK_RUNNING                0
// #define TASK_INTERRUPTIBLE       1
// #define TASK_UNINTERRUPTIBLE     2
// #define TASK_ZOMBIE              3
// #define TASK_STOPPED             4

#define PREEMPT_ENABLE  0
#define PREEMPT_DISABLE 1

/* Lab3中进程的数量以及每个进程初始的时间片 */
#define LAB_TEST_NUM        4
#define LAB_TEST_COUNTER    5  

/* 当前进程 */
extern struct task_struct *current;

/* 进程指针数组 */
extern struct task_struct * task[NR_TASKS];

/* 进程状态段数据结构 */
struct thread_struct {
    unsigned long long ra;
    unsigned long long sp;
    unsigned long long s0;
    unsigned long long s1;
    unsigned long long s2;
    unsigned long long s3;
    unsigned long long s4;
    unsigned long long s5;
    unsigned long long s6;
    unsigned long long s7;
    unsigned long long s8;
    unsigned long long s9;
    unsigned long long s10;
    unsigned long long s11;
};

/* 进程数据结构 */
struct task_struct {
     long state;    // 进程状态 Lab3中进程初始化时置为TASK_RUNNING
     long counter;  // 剩余运行时间 
     long priority; // 运行优先级 1最高 5最低
     long blocked;
     long pid;      // 进程标识符
    // Above Size Cost: 40 bytes
    struct thread_struct thread; // 该进程状态段
};

/* 进程初始化 创建四个dead_loop进程 */ 
void task_init(void); 

/* 在时钟中断处理中被调用 */
void do_timer(void);

/* 调度程序 */
void schedule(void);

/* 切换当前任务current到下一个任务next */
void switch_to(struct task_struct* next);

/* 死循环 */
void dead_loop(void);

#endif
```

  
