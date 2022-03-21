# Lab 6：RISC-V 动态内存分配与缺页异常处理

## **1.**   实验目的

在充分理解前序实验中RISC-V地址空间映射机制与任务调度管理的前提下，进一步了解与**动态**内存管理相关的重要结构，实现基本的内存管理算法，并最终能够较为**综合**地实现对进程的内存管理与调度管理。

## **2.**   实验目标

+ 目标一：了解**Buddy System** 和**Slub Allocator** 物理内存管理机制的实现原理，并用**Buddy System** 配合**Slub Allocator** 管理和分配物理内存，最终实现统一的内存分配/释放接口:**kmalloc**/**kfree**。
+ 目标二：在**mm_struct**中补充实现**vm_area_struct**数据结构的支持，并结合后续**mmap**等系统调用实现对进程**多区域**虚拟内存的管理。
+ 目标三：实现**222: mmap, 215: munmap, 226: mprotect**系统调用。
+ 目标四：在**Lab5** 实现用户态程序的基础上，添加缺页异常处理**Page Fault Handler**，并在分配物理页或物理内存区域的同时更新页表。
+ 目标五：综合上述实现，为进程加入**fork** 机制，能够支持创建新的用户态进程，并测试运行。

## 3. 实验环境

- Docker Image  `docker exec -it  -u oslab -w /home/oslab reverent_noyce /bin/bash`

## 4. 背景知识

### 4.1 物理内存动态分配算法

#### 4.1.1 伙伴系统（Buddy System）

操作系统内核可采用伙伴系统（Buddy System）对物理内存进行管理。伙伴系统有多种，最常见的为依次按2的幂次大小组成的二进制伙伴系统（Binary Buddy System），这也是Linux系统中目前所采用的一种物理内存管理算法

1. Binary buddy system <--本次试验所采用的伙伴系统算法
2. Fibonacci buddy system
3. Weighted buddy system
4. Tertiary buddy system

把所有的空闲页框分组为11个块链表，每个块链表分别包含大小为1，2，4，8，16，32，64，128，256，512和1024个连续页框的页框块。最大可以申请1024个连续页框，对应4MB大小的连续内存。每个页框块的第一个页框的物理地址是该块大小的整数倍.

#### 4.1.2 SLAB/SLUB分配器

Buddy System以2^n个page为粒度来进行物理内存的分配管理，Linux中实现的Buddy System最小阶（order）为2^0，即最小4KB，最大阶为2^10，也就是4MB大小的连续物理内存空间。但是Buddy System并不满足更细粒度的内存管理，当由于分配/释放的内存空间与最小存储空间单位(1-Page)不完全相符时，如在先前实验中实现过的**task struct**(该数据结构的大小**小于**4KB)，该类小空间内存的频繁分配/释放容易产生**内部碎片**。为此需要借助专门的内存分配器加以优化解决**内部碎片**的问题。

SLAB是一个通用名称，指的是使用对象缓存的内存分配策略，从而能够有效地分配和释放内核对象。在Solaris 2.4内核中实现。Linux目前为它的“Slab”分配器提供了三种选择: SLAB是最初的版本，基于Bonwick的开创性论文，从Linux内核版本2.2开始就可以使用了。在Bonwick的后续[论文](http://static.usenix.org/event/usenix01/full_papers/bonwick/bonwick.pdf)中描述了多处理器的改变。

SLUB是下一代内存分配器（自Linux 2.6.23以来一直是Linux内核中的默认配置）。它继续采用基本的“SLAB”模型，但修复了SLAB设计中的一些缺陷，特别是在拥有大量处理器的系统上。且SLUB甚至要比SLAB简单。SLUB机制[【link】](http://www.wowotech.net/memory_management/426.html)，可以理解为将Page拆分为更小的单位来进行管理。SLUB系统的核心思想是使用**对象**的概念来管理物理内存。

#### 4.1.3 SLUB & Buddy System

本次试验中，SLUB直接给出，学生需要自己实现Buddy System 以来支持 SLUB 功能，下面介绍下SLUB 与 Buddy System的依赖关系：

* Buddy System 是以**页(Page)**为基本单位来分配空间的，其提供两个接口**alloc_pages(申请若干页)** 和**free_pages(释放若干页)**
* SLUB在初始化时，需要预先申请一定的空间来做数据结构和Cache的初始化，此时依赖于Buddy System提供的上述接口。（具体可以参考slub.c中的**slub_init**）
* 实验所用的slub提供8, 16, 32, 64, 128, 256, 512, 1024, 2048（单位：Byte）九种object-level（的内存分配/释放功能。(具体可以参考slub.c中的**kmem_cache_alloc /  kmem_cache_free**)
* slub.c 中的(**kmalloc / kfree**) 提供内存动态分配/释放的功能。根据请求分配的空间大小，来判断是通过kmem_cache_alloc来分配object-level的空间，还是通过alloc_pages来分配page-level的空间。kfree同理。那么怎么判断呢?  cache 是啥,  alloc 又是啥?    就是很大的时候用buddy alloc, 小的时候用slub. cache 就是存了一些内存对象, alloc 就是分配

### 4.2 vm_area_struct 介绍

vm area struct 简称 VMA, 每个segment用一个VMA 结构体表示。vma是通过一个双向链表（早期的内核实现是单向链表）串起来的，现存的vma按起始地址以递增次序被归入链表中，每个vma是这个链表里的一个节点。

在先前的实验中，我们已经在 `task_struct`中添加了储存虚拟内存映射相关信息的 `mm_struct`数据结构，而并未对虚拟地址空间的分配情况进行设置与保存。在linux系统中，`vm_area_struct`是虚拟内存管理的基本单元，`vm_area_struct`保存了有关连续虚拟内存区域(简称vma)的信息。linux 具体某一进程的虚拟内存区域映射关系可以通过 [procfs【Link】](https://man7.org/linux/man-pages/man5/procfs.5.html) 读取 `/proc/pid/maps`的内容来获取:

比如，如下一个常规的 `bash`进程，假设它的进程号为 `7884`，则通过输入如下命令，就可以查看该进程具体的虚拟地址内存映射情况(部分信息已省略)。

```shell
#cat /proc/7884/maps
556f22759000-556f22786000 r--p 00000000 08:05 16515165                   /usr/bin/bash
556f22786000-556f22837000 r-xp 0002d000 08:05 16515165                   /usr/bin/bash
556f22837000-556f2286e000 r--p 000de000 08:05 16515165                   /usr/bin/bash
556f2286e000-556f22872000 r--p 00114000 08:05 16515165                   /usr/bin/bash
556f22872000-556f2287b000 rw-p 00118000 08:05 16515165                   /usr/bin/bash
556f22fa5000-556f2312c000 rw-p 00000000 00:00 0                          [heap]
7fb9edb0f000-7fb9edb12000 r--p 00000000 08:05 16517264                   /usr/lib/x86_64-linux-gnu/libnss_files-2.31.so
7fb9edb12000-7fb9edb19000 r-xp 00003000 08:05 16517264                   /usr/lib/x86_64-linux-gnu/libnss_files-2.31.so               
...
7ffee5cdc000-7ffee5cfd000 rw-p 00000000 00:00 0                          [stack]
7ffee5dce000-7ffee5dd1000 r--p 00000000 00:00 0                          [vvar]
7ffee5dd1000-7ffee5dd2000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
```

从中我们可以读取如下一些有关该进程内虚拟内存映射的关键信息：

+ `vm_start`&ensp;:  (第1列) 指的是该段虚拟内存区域的开始地址；
+ `vm_end`&ensp;&ensp;&ensp;:  (第2列) 指的是该段虚拟内存区域的结束地址；
+ `vm_flags`  :  (第3列) 该`vm_area`的一组权限(rwx)标志，`vm_flags`的具体取值定义可参考linux源代码的[这里(linux/mm.h)](https://elixir.bootlin.com/linux/latest/source/include/linux/mm.h#L250)
+ `vm_pgoff`  :  (第4列) 虚拟内存映射区域在文件内的偏移量；
+ `vm_file`&ensp;&ensp;:  (第5/6/7列)分别表示：映射文件所属设备号/以及指向关联文件结构的指针(如果有的话，一般为文件系统的inode)/以及文件名；

其它保存在 `vm_area_struct`中的信息还有：

+ `vm_ops`&ensp;&ensp;&ensp;:  该 `vm_area`中的一组工作函数;
+ `vm_next/vm_prev`: 同一进程的所有虚拟内存区域由**链表结构**链接起来，这是分别指向前后两个`vm_area_struct`结构体的指针;

### 4.3 文件/内存映射 系统调用

沿用Lab5中添加syscall的方法，本次实现的系统调用函数原型以及具体功能如下：

#### 4.3.1 实现 222 号系统调用：mmap. (memory map)

```c
void *mmap (void *__addr, size_t __len, int __prot,
                   int __flags, int __fd, __off_t __offset)
```

+ 说明：在Linux中，使用 `mmap`在进程的虚拟内存地址空间中分配新地址空间（从 `void *__addr`开始, 长度为 `size_t __len`字节），创建和物理内存的映射关系。它可以分配一段匿名的虚拟内存区域，也可以映射一个文件到内存。利用该特性，`mmap()`系统调用使得进程之间通过映射同一个普通文件实现共享内存成为可能。普通文件被映射到进程虚拟地址空间后，进程可以向访问普通内存一样对文件进行访问，而不必再调用 `read()`，`write()`等函数。mmap()其实只是将一个文件的一部分内容映射到了进程虚拟地址空间中的一个[VMA区域](https://zhuanlan.zhihu.com/p/67936075)，并没有真正分配物理内存，只有等到进程真正访问这个VMA的时候，才会触发[page fault](https://www.zhihu.com/search?q=page+fault&search_source=Entity&hybrid_search_source=Entity&hybrid_search_extra={"sourceType"%3A"article"%2C"sourceId"%3A"67894878"})，将这部分文件内容从磁盘拷贝到内存中。
+ 参数 `int __prot `表示了对所映射虚拟内存区域的权限保护要求。利用如下所示不同的 `int __prot`取值可设置不同的内存保护权限：

| Defined `__prot`  Symbol | Value      | Description                                   |
| -------------------------- | ---------- | --------------------------------------------- |
| PROT_NONE                  | 0x0        | 页内容不可被访问                              |
| PROT_READ                  | 0x1        | 页内容可以被读取                              |
| PROT_WRITE                 | 0x2        | 页可以被写入内容                              |
| PROT_EXEC                  | 0x4        | 页内容可以被执行                              |
| PROT_SEM                   | 0x8        | (可选) 页面可能用于原子操作(atomic operation) |
| PROT_GROWSDOWN             | 0x01000000 | (可选) /                                      |
| PROT_GROWSUP               | 0x02000000 | (可选) /                                      |

+ 参数 `int __flags`  (可选) 指定了映射对象的类型，具体的参数设置可以参考[这里【link】](https://man7.org/linux/man-pages/man2/mmap.2.html)
+ 参数 `int __fd`  (可选) 为即将映射到进程空间的文件描述字，一般由 `open()`返回，同时，`__fd`可以指定为 `-1`，此时须指定 `__flags`参数中的MAP_ANON，表明进行的是匿名映射。`mmap`的作用是映射文件描述符 `__fd`指定文件的 ` [off,off + len]`区域至调用进程的 `[addr, addr + len]`的虚拟内存区域。在Lab7 ELF文件的载入运行实验中，可以实现进程之间通过映射同一个普通文件实现共享内存。
+ 参数 `__off_t __offset`  (可选) 该参数一般设为 `0`，表示从文件头开始映射。
+ 调用 mmap `返回：被映射内存区域的指针`

#### 4.3.2 实现 226 号系统调用：mprotect. (memory protect)

```c
int mprotect (void *__addr, size_t __len, int __prot)
```

+ 说明：在Linux中，使用 `mprotect`函数可以用来修改一段指定内存区域（从 `void *__addr`开始, 长度为 `size_t __len`字节）的内存保护权限属性：`int __prot`。
+ 调用 mprotect `返回：0`   	表示 `mprotect`成功;  `返回：-1`  	表示 `mprotect`出错;

#### 4.3.3 实现 215 号系统调用：munmap. (memory unmap)

```c
int munmap (void *__addr, size_t __len)
```

+ 说明：在Linux中，使用 `munmap` 解除从 `void *__addr`开始, 长度为 `size_t __len`字节区域的任何内存映射。
+ 调用 munmap `返回：0`   	表示munmap成功;  `返回：-1`  	表示munmap出错;

### 4.4 缺页异常 Page Fault

缺页异常是一种正在运行的程序访问当前未由内存管理单元（MMU）映射到虚拟内存的页面时，由计算机硬件引发的异常类型。访问未被映射的页或访问权限不足，都会导致该类异常的发生。处理缺页异常通常是操作系统内核的一部分。当处理缺页异常时，操作系统将尝试使所需页面在物理内存中的位置变得可访问（建立新的映射关系到虚拟内存）。而如果在非法访问内存的情况下，即发现触发Page Fault的虚拟内存地址(Bad Address)不在当前进程 `vm_area_struct`链表中所定义的允许访问的虚拟内存地址范围内，或访问位置的权限条件不满足时，缺页异常处理将终止该程序的继续运行。

#### 4.4.1 RISC-V Page Faults

RISC-V 异常处理：当系统运行发生异常时，可即时地通过解析csr scause寄存器的值，识别如下三种不同的Page Fault。

**SCAUSE** 寄存器指示发生异常的种类：

| Interrupt/Exception`<br>`scause[XLEN-1] | Exception Code`<br>`scause[XLEN-2:0] | Description            |
| ----------------------------------------- | -------------------------------------- | ---------------------- |
| 0                                         | 12                                     | Instruction Page Fault |
| 0                                         | 13                                     | Load Page Fault        |
| 0                                         | 15                                     | Store/AMO Page Fault   |

#### 4.4.2 常规处理**Page Fault**的方式介绍

处理缺页异常时所需的信息如下：

+ 1. 触发**Page Fault**时访问的虚拟内存地址VA (Virtual Address)，也称为Bad Address
     + 当触发page fault时，csr STVAL寄存器被被硬件自动设置为该出错的VA地址。
+ 2. 导致**Page Fault**的类型
     + Exception Code = 12: page fault caused by an instruction fetch
     + Exception Code = 13: page fault caused by a read
     + Exception Code = 15: page fault caused by a write
+ 3. 发生**Page Fault**时的指令执行位置以及当前程序所运行的权限模式(User/Kernel) (可选)
+ 4. 保存在当前进程**task_struct->mm_struct**中合法的**VMA**映射关系
     + 保存在`vm_area_struct`链表中

### 4.5 `fork`系统调用

在之前的lab3中，我们创建了四个进程。现在我们需要在lab5用户态支持和lab6之前实现的基础上实现 `fork`系统调用。

#### 4.5.1 实现 220 号系统调用：fork (clone) 

```c
pid_t fork（void)
```

* `fork()`通过复制当前进程创建一个新的进程，新进程称为子进程，而原进程称为父进程。
* 子进程和父进程在不同的内存空间上运行，在其中一个进程中调用 `mmap`或 `munmap`系统调用不会影响另一进程的内存空间。
* 父进程 `fork`成功时 `返回：子进程的pid`，子进程 `返回：0`。`fork`失败则父进程 `返回：-1`。
* 本系统调用创建的子进程需要拷贝父进程 `task_struct`、`根页表`、`mm_struct`以及父进程的 `用户栈`等信息。
* 本系统调用实现时拷贝了父进程的全部内存空间，这是非常耗时的行为。Linux中使用了写时复制（copy-on-write）机制，fork创建的子进程首先与父进程共享物理内存空间，直到父子进程有修改内存的操作发生时再为子进程分配物理内存。

思考题：`fork`系统调用为什么可以有两个返回值？

 **父进程和子进程都要执行，返回值也不一样**，所以fork()API**表现为一次调用两次返回**，但**实质任何函数调用都只会返回一次，父进程是返回，而子进程只是从父进程的返回指令处开始执行**。

## 5. 实验步骤

### 5.1 环境搭建

#### 5.1.1 建立映射

同lab5的文件夹映射方法，目录名为lab6。

#### 5.1.2 组织文件结构

请同学们参考如下结构为lab6组织文件目录：其中 `slub.h`和 `slub.c`由实验预先给出

```
lab6
├── arch
│   └── riscv
│       ├── include
│       │   └── ...
│       ├── kernel
│       │   │
│       │  ...
│       │   ├── slub.c 
│       │   ├── buddy.c
│       │   └── fault.c
│       └── Makefile
├── include
│   └── ...
├── init
│   └── ...
├── lib
│   └── ...
└── Makefile
```

### 5.2 管理空闲内存空间 ，在 buddy.c 中实现 buddy system

#### 5.2.1 相关数据结构以及接口

```c
struct buddy {
  unsigned long size;
  unsigned *bitmap; 
};

void init_buddy_system(void);
void *alloc_pages(int);
void free_pages(void*);
```

- size：buddy system 管理的**页**数
- bitmap：用来记录未分配的内存页信息
- void init_buddy_system(void)：初始化buddy system
- void *alloc_pages(int npages)：申请n个page空间，返回起始虚拟地址
- void free_pages(void* addr)：释放以addr为基地址的空间块

##### init_buddy_system：理解下图，并按照后续指导完成buddy system的初始化

```c
假设我们系统一共有8个可分配的页面，可分配的页面数需保证是2^n
memory: 
+--------+--------+--------+--------+--------+--------+--------+--------+
| page 0 | page 1 | page 2 | page 3 | page 4 | page 5 | page 6 | page 7 |
+--------+--------+--------+--------+--------+--------+--------+--------+
buddy system将页面整理为 2^n 2^(n-1) ... 2^1 2^0 不同大小的块：
8 +---> 4 +---> 2 +---> 1
  |       |       |
  |       |       +---> 1
  |       |
  |       +---> 2 +---> 1
  |               |
  |               +---> 1
  |
  +---> 4 +---> 2 +---> 1
          |       |
          |       +---> 1
          |
          +---> 2 +---> 1
                  |
                  +---> 1
当申请空间时候，buddy system 会将满足要求的最小块分配出去。上图是满二叉树结构，所以我们可以很方便的使用数组来保存相关信息。如下：
buddy.size = 8
buddy.bitmap:
+-----------------------------------------------------------+
| 8 | 4 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
+-----------------------------------------------------------+
bitmap中的 数字 代表的 可分配的物理地址连续页的最大个数 ， 比如此时：
bitmap[0] 对应的是 page 0 ~ page 7
bitmap[1] 对应的是 page 0 ~ page 3
bitmap[2] 对应的是 page 4 ~ page 7
bitmap[3] 对应的是 page 0 ~ page 1
bitmap[4] 对应的是 page 2 ~ page 3
... 以此类推
```

* 实现的相关细节
  * 计算出总共物理页面。（提示： 查看 PA 的起始 结束地址）
  * 完成对 buddy 数据结构的初始化。（提示：用数组表示满二叉树，parent 节点的index 与 child节点的index 存在一一对应的关系）
  * 由于kernel的代码占据了一部分空间，在初始化结束后，需要将这一部分的空间标记为已分配。(提示：可以直接使用alloc_pages 将这段空间标记为已分配)
  * 这里需要注意bitmap 与 page X 的对应关系。（提示 ：查看((index + 1) * buddy.bitmap[index] - buddy.size)) 与 page X 的关系
  * 将 init_buddy_system 添加至**建立内核映射**之前， 修改之前实现的物理页分配逻辑

那么, 我们需要多大的size呢?  bitmap多大呢? 

size 就是 1000 

```c
#define P_PAGE 0x1000
#define parent(x) (((x + 1) >> 1) - 1)
#define lson(x) (((x + 1) << 1) - 1)
#define rson(x) ((x + 1) << 1)

struct buddy buddy;
unsigned ins_bitmap[2 * P_PAGE - 1];
unsigned long page_offset = 0x80000000ul;

extern unsigned long *_end;
```

P_PAGE为总⻚数，16M/4K = 0x1000 

parent、lson、rson分别为计算⽗亲、左⼉⼦和右⼉⼦的标号 

ins_bitmap是定义在程序字段的bitmap，buddy.bitmap指向这个数组的开头 

page_offset是决定alloc_pages返回的地址的偏移量，起始置为0x80000000，alloc_pages返回物理地址，当paging_init结束后置page_offset为0xffffffe000000000，alloc_pages返回虚拟地址。 

_end为从lds中引⼊的标签

```c
void init_buddy_system(void) {
    buddy.size = P_PAGE;
    buddy.bitmap = ins_bitmap;
    unsigned long x = buddy.size;
    unsigned long y = 1;
    unsigned long i = 0;
    for (  ; x > 0; x >>= 1, i += y, y <<= 1) {  
        for (int j = 0; j < y; ++j) {
            ins_bitmap[i + j] = x;
        }
    }
    //由于kernel的代码占据了一部分空间，在初始化结束后，需要将这一部分的空间标记为已分配
    alloc_pages((((unsigned long)&_end - 0x80000000ul) >> 12));
}
```

##### alloc_pages：理解下图，并完成alloc_pages接口

设某个节点索引值为index,则节点的左子节点索引为:  2*index+1 . 利用marco 来快速计算

右子节点索引为: 2*index+2

父节点索引为:  (index-1)/2  或者(((x + 1) >> 1) - 1)

```c
memory: （这里的 page 0～7 计做 page X）
+--------+--------+--------+--------+--------+--------+--------+--------+
| page 0 | page 1 | page 2 | page 3 | page 4 | page 5 | page 6 | page 7 |
+--------+--------+--------+--------+--------+--------+--------+--------+
buddy.bitmap:
+-----------------------------------------------------------+
| 8 | 4 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
+-----------------------------------------------------------+
alloc_pages(3) 请求分配3个页面 由于buddy system每次必须分配2^n个页面，所以我们将 3 个页面向上扩展至 4 个页面。 full_size 有什么用?  
我们从树根出发（buddy.bitmap[0]），查找到恰好满足当前大小需求的节点： 
        ↓
8 +---> 4 +---> 2 +---> 1
  |       |       |
  |       |       +---> 1
  |       |
  |       +---> 2 +---> 1
  |               |
  |               +---> 1
  |
  +---> 4 +---> 2 +---> 1
          |       |
          |       +---> 1
          |
          +---> 2 +---> 1
                  |
                  +---> 1  
  index = 1
      ↓  
+-----------------------------------------------------------+
| 8 | 4 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
+-----------------------------------------------------------+
通过 index 找到对应 page X，再通过适当计算最终转化为VA
更新bitmap： 
↓       ↓
4 +---> 0 +---> 2 +---> 1
  |       |       |
  |       |       +---> 1
  |       |
  |       +---> 2 +---> 1
  |               |
  |               +---> 1
  |
  +---> 4 +---> 2 +---> 1
          |       |
          |       +---> 1
          |
          +---> 2 +---> 1
                  |
                  +---> 1  
  ↓   ↓
+-----------------------------------------------------------+
| 4 | 0 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
+-----------------------------------------------------------+
由于 index = 1 对应的 4 个 page已经分配出去，所以 bitmap[1] = 0
无需再去更新 index = 1的所有子节点了，因为我们的查找是从父节点开始的， 当一个节点的可分配页面数为0，就不会去查找其子节点了。
同理，由于 bitmap[0] 所对应的 8 个物理地址连续页， 其中的 4 个被分配出去了， 所以bitmap[0]对应的可分配物理地址连续的页数为 4 个。
```

```c

void *alloc_pages(int n) {
    for (int offset = 0; ; ++offset) {
        if ((1 << offset) >= n) {
            n = (1 << offset);
            break;// 每次必须分配2^n个页面 
        }
    }
    if (ins_bitmap[0] < n) {
        return 0;
    }
    int i = 0;
    int full_size = buddy.size;
    unsigned t_bitmap;
    //查找恰好满足当前大小需求的节点
    while (i < 2 * P_PAGE - 1) {
        if (ins_bitmap[i] == n && ins_bitmap[i] == full_size) {
            t_bitmap = ins_bitmap[i];
            ins_bitmap[i] = 0;
            int ii = i;
            while (ii > 0) {
                ii = parent(ii);
                ins_bitmap[ii] = ins_bitmap[rson(ii)] > ins_bitmap[lson(ii)] ? 
                        ins_bitmap[rson(ii)] : ins_bitmap[lson(ii)];//更新bitmap
            }
            break;
        } else {
            if (ins_bitmap[lson(i)] >= n) {
                full_size >>= 1;
                i = lson(i);
            } else if (ins_bitmap[rson(i)] >= n) {
                full_size >>= 1;
                i = rson(i);
            }
        }
    }
    // 通过 index 找到对应 page X，再通过适当计算最终转化为VA
    //注意使⽤page_offset来控制返回的地址是物理地址还是虚拟地址。
    unsigned long addr = (((i + 1) * t_bitmap - buddy.size) << 12) + page_offset;
    if (addr >= 0xffffffe000000000) {
        puts("[S] Buddy allocate addr: ");
        puti64(addr);
        puts("\n");
    }
    return addr;
}  
```

##### free_pages：理解下图，并完成free_pages接口

```c
我们考虑 alloc_pages(1) 之后的状态：
4 +---> 2 +---> 2 +---> 1
  |       |       |
  |       |       +---> 1
  |       |           
  |       +---> 1 +---> 0 ← 
  |               |
  |               +---> 1
  |
  +---> 4 +---> 2 +---> 1
          |       |
          |       +---> 1
          |
          +---> 2 +---> 1
                  |
                  +---> 1
buddy.bitmap:
                              ↓
+-----------------------------------------------------------+
| 4 | 2 | 4 | 1 | 2 | 2 | 2 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
+-----------------------------------------------------------+

现在对这个页面进行free_pages的操作：
通过 va 进行适当的转换，我们可以得到该页面对应的 bitmap index，将其更新为正确的值，并更新其祖先的值。
更新其祖先的值的时候，需要去判断是否需要对祖先的左右两个孩子进行合并
若两个孩子都未被分配则合并，否则不合并，修改其值为 左右两个孩子中最大物理连续页的数量。具体过程如下图：
                            index = 7
                              ↓
+-----------------------------------------------------------+
| 4 | 2 | 4 | 1 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |  修改 bitmap[7] = 1
+-----------------------------------------------------------+
                                  
            index = 3
              ↓
+-----------------------------------------------------------+
| 4 | 2 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |  修改 bitmap[4], 由于其左右两个孩子都
+-----------------------------------------------------------+  未被分配，故需要合并 bitmap[4] = 2

    index = 1
      ↓
+-----------------------------------------------------------+
| 4 | 4 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |  修改 bitmap[1], 同理 bitmap[1] = 4
+-----------------------------------------------------------+

index = 0
  ↓
+-----------------------------------------------------------+
| 4 | 4 | 4 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |  修改 bitmap[0], 同理 bitmap[0] = 8
+-----------------------------------------------------------+
```

```c

void free_pages(void* x) {
    unsigned long index = x;
    index -= page_offset;
    index >>= 12;
    index += buddy.size;
    //通过 va 进行适当的转换，我们可以得到该页面对应的 bitmap index

    int i = 0;
    int full_size = buddy.size;
    while (i < 2 * P_PAGE - 1) {
        if ((i + 1) * ins_bitmap[i] == index) {
            if (ins_bitmap[i] == 0) {
                ins_bitmap[i] = full_size;
                int ii = i;
                int t_full_size = full_size;
                while (ii > 0) {
                    ii = parent(ii);
                    t_full_size <<= 1;
                    ////判断是否需要对祖先的左右两个孩子进行合并
                    if (ins_bitmap[lson(ii)] == (t_full_size >> 1) && ins_bitmap[rson(ii)] == (t_full_size >> 1)) {
                        ins_bitmap[ii] = t_full_size;//两个孩子都未被分配则合并
                    } else {
                        //否则不合并，修改其值为 左右两个孩子中最大物理连续页的数量
                        ins_bitmap[ii] = ins_bitmap[rson(ii)] > ins_bitmap[lson(ii)] ? 
                                ins_bitmap[rson(ii)] : ins_bitmap[lson(ii)];
                    }
                }
            } else {
                full_size >>= 1;
                i = lson(i);
            }
        } else {
            full_size >>= 1;
            i = rson(i);
        }
    }
}
```

### 5.3 在 slub.c 中实现 slub 内存动态分配算法

+ 同学们需要在实现Buddy System接口的基础上，进一步实现SLUB分配器
+ SLUB分配器的代码**已给出（见代码仓库lab6目录）**，同学们只需将SLUB分配器移植到自己的实验项目中即可

SLUB分配器提供的接口说明如下：

#### 5.3.1  slub接口: kmem_cache_create

```
struct kmem_cache *
kmem_cache_create(const char *name, unsigned int size, unsigned int align,
		slab_flags_t flags, void (*ctor)(void *))
```

#### 5.3.2 slub接口: kmem_cache_destroy

参考 linux-5.9.12/mm/slab_common.c

```
void kmem_cache_destroy(struct kmem_cache *s)
```

#### 5.3.3 slub接口: kmem_cache_alloc

参考 linux-5.9.12/mm/slub.c

Target: Allocate an object from this cache.  The flags are only relevant if the cache has no available objects.

Return: pointer to the new object or NULL in case of error

```
void *kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags)
```

#### 5.3.4 slub接口: kmem_cache_free

参考 linux-5.9.12/mm/slub.c

Target：Free an object which was previously allocated from this cache.

```
void kmem_cache_free(struct kmem_cache *cachep, void *objp)
```

### 5.4 实现统一内存分配接口

#### 5.4.1  slub 中页面的属性，相关数据结构：

在实现内存分配接口之前，需要介绍下 slub 中页面的属性，相关数据结构如下：

```c
// slub.c
enum{
	PAGE_FREE,
	PAGE_BUDDY,
	PAGE_SLUB,
	PAGE_RESEARVE
};

// slub.h
struct page {
	unsigned long flags;
	int count;	
	struct page *header;
	struct page *next;
	struct list_head slub_list;
	struct kmem_cache *slub;
	void *freelist;
};
```

其中 page 中 flags 用来表示当前page的类别，因此 如果一个页面是用来做object-level内存分配那么它的flag 应设置为 **PAGE_SLUB**， 如果是通过Buddy System(alloc_pages)分配的，那么其flags应设置为 **PAGE_BUDDY**。

同学需要弄清楚 **set_page_attr / clear_page_attr / cache_alloc_pages / ADDR_TO_PAGE / PAGE_TO_ADDR** 相关代码实现逻辑

#### 5.4.2 实现内存分配接口 void *kmalloc（size_t size)

该函数的功能是申请大小为 size 字节的连续物理内存，成功返回起始地址，失败返回 NULL。

+ 每个 slub allocator 分配的对象大小见**kmem_cache_objsize**。
+ 当请求的物理内存**小于等于**2^11字节时，从`slub allocator` 中分配内存。
+ 当请求的物理内存大小**大于**2^11字节时，从`buddy system` 中分配内存。

```c

void *kmalloc(size_t size) {
  int objindex;
  void *p;
  if (size == 0)
    return NULL;
  // size 若在 kmem_cache_objsize 所提供的范围之内，则使用 slub allocator
  // 来分配内存
  for (objindex = 0; objindex < NR_PARTIAL; objindex++) {
    if (size <= kmem_cache_objsize[objindex]) {
      p = kmem_cache_alloc(slub_allocator[objindex]);
      break;
    }
    // YOUR CODE HERE
  }
  // size 若不在 kmem_cache_objsize 范围之内，则使用 buddy system 来分配内存
  if (objindex >= NR_PARTIAL) {
    // YOUR CODE HERE
    p = alloc_pages(size >> 12);
    set_page_attr(p, (size - 1) / PAGE_SIZE, PAGE_BUDDY);
  }
  return p;
}
```

#### 5.4.2 实现内存分配接口 void kfree(void *addr)

```c
void kfree(const void *addr) {
  struct page *page;
  if (addr == NULL)
    return;
  // 获得地址所在页的属性
  // YOUR CODE HERE
  page = ADDR_TO_PAGE(addr);
  // 判断当前页面属性
  if (page->flags == PAGE_BUDDY) {
    // YOUR CODE HERE
    free_pages(addr);
    clear_page_attr(ADDR_TO_PAGE(addr)->header);

  } else if (page->flags == PAGE_SLUB) {
    // YOUR CODE HERE
    kmem_cache_free(addr);
  }
  return;
}
```

#### 5.4.3 修改之前代码中的页面分配逻辑，使用 **alloc_pages / free_pages / kmalloc / kfree** 来进行页面分配和释放

### 5.5 为 **mm_struct** 添加 **vm_area_struct** 数据结构

### 5.5.1 定义 `vm_area_struct` 数据结构： 

在sched.h 文件.

```c++
typedef struct { unsigned long pgprot; } pgprot_t;
struct vm_area_struct {
    /* Our start address within vm_area. */
    unsigned long vm_start;	
    /* The first byte after our end address within vm_area. */
    unsigned long vm_end;	
    /* linked list of VM areas per task, sorted by address. */
    struct vm_area_struct *vm_next, *vm_prev;
    /* The address space we belong to. */
    struct mm_struct *vm_mm;
    /* Access permissions of this VMA. */
    pgprot_t vm_page_prot;
    /* Flags*/
    unsigned long vm_flags;	
};
struct mm_struct {
    unsigned long *pgtbl;
    unsigned long text_start;
    unsigned long text_end;
    unsigned long stack_top;
    unsigned long pa_for_stack;
    struct vm_area_struct *vm_area_head;
};
```

### 5.5.2 `vm_page_prot`和 `vm_flags`的区别

我们可以注意到这两个字段的C语言类型。`vm_page_prot`的类型是 `pgprot_t`，这是arch级别的数据类型，这意味着它可以直接应用于底层架构的Page Table Entries。在RISC-V 64位上，这个字段直接存储了vma的pte中保护位的内容。而 `vm_flags`是一个与arch无关的字段，它的位是参照linux/mm.h中定义的。可简单地将 `vm_flags`看作是 `vm_page_prot`的翻译结果，方便在操作系统其它地方的代码中判断每个 `vm_area`的具体权限。

+ 其中vm_flags的取值需要采用与[这里【link】](https://elixir.bootlin.com/linux/latest/source/include/linux/mm.h#L250)一致的定义方式。
+ 其余结构体内具体的成员变量含义已在 `4.2节`中描述。

建立上述 `vm_area_struct`数据结构后，需配合 `mmap`等系统调用为进程初始化一个合理的 `vm_area_struct`链表，使得后续进程在运行时可同时管理多个 `vm_area`。

### 5.6 实现mmap/munmap/mprotect的SysCall

#### 5.6.1 实现mmap函数

在Linux中，`mmap`的功能已在4.2.1中阐述。在本次实验中 `mmap`的功能有所简化，不涉及到文件映射，而是负责分配对应长度的若干个匿名内存页区域，并将这段内存区域的信息插入到当前进程 `current`中的 `vm_area_struct`中，作为链表的一个新节点。具体的函数定义及参数含义如下：

```C
void *mmap (void *__addr, size_t __len, int __prot,
                   int __flags, int __fd, __off_t __offset)
```

- `size_t`与`__off_t`为`unsigned long`的类型别名
- `__addr`:**建议**映射的虚拟首地址，需要按页对齐
- `__len`: 映射的长度，需要按页对齐
- `__prot`: 映射区的权限，在4.3.1节中已说明
- `__flags`: 由于本次实验不涉及`mmap`在Linux中的其他功能，该参数无意义，固定为`MAP_PRIVATE | MAP_ANONYMOUS`
  - `MAP_PRIVATE`表示该映射是私有的，值为`0x2`
  - `MAP_ANONYMOUS`表示映射对象为匿名内存页，值为`0x20`
- `__fd`: 由于本次实验不涉及文件映射，该参数无意义，固定为`-1`
- `__offset`: 由于本次实验不涉及文件映射，该参数无意义，固定为`0`
- 返回值：**实际**映射的虚拟首地址，若虚拟地址区域`[__addr, __addr+__len)`未与其它地址映射冲突，则返回值就是建议首地址`__addr`，若发生冲突，则需要更换到无冲突的虚拟地址区域，返回值为该区域的首地址

`mmap`映射的虚拟地址区域采用需求分页（Demand Paging）机制，即：`mmap`中不为该虚拟地址区域分配实际的物理内存，仅当进程试图访问该区域时，通过触发Page Fault，由Page Fault处理函数进行相应的物理内存分配以及页表的映射。

```c

void *do_mmap(struct mm_struct *mm, void *start, size_t length, int prot) {
    struct vm_area_struct *ptr;
    //检查建议的虚拟地址区域`[start, start+length)`是否与已有映射冲突，若冲突，则调用`get_unmapped_area`生成新的地址区域
    for (ptr = mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
        if (ptr->vm_start < (unsigned long)start + length && ptr->vm_end > (unsigned long)start) {
            start = get_unmapped_area(length);
            break;
        }
    }
    //配合需求分页机制，需要创建一个`vm_area_struct`记录该虚拟地址区域的信息，并添加到`mm`的链表中
    struct vm_area_struct *new_vm_area = kmalloc(sizeof(struct vm_area_struct));
    new_vm_area->vm_start = (unsigned long)start;
    new_vm_area->vm_end = start + length;
    new_vm_area->vm_flags = prot;
    new_vm_area->vm_mm = mm;
    new_vm_area->vm_page_prot.pgprot = ((prot & PROT_READ ? 1 : 0) | (prot & PROT_WRITE ? 2 : 0) | (prot & PROT_EXEC ? 4 : 0)) << 1;
    //将该`vm_area_struct`插入到`mm`的链表中
    if (mm->vm_area_head == NULL) {
        mm->vm_area_head = new_vm_area;
    } else {
        for (ptr = mm->vm_area_head; ptr->vm_next != NULL; ptr = ptr->vm_next) ;
        ptr->vm_next = new_vm_area;
        new_vm_area->vm_prev = ptr;
    }
    return start;
}

//分配对应长度的若干个匿名内存页区域，并将这段内存区域的信息插入到当前进程 `current`中的 `vm_area_struct`中，作为链表的一个新节点
void *mmap (void *__addr, size_t __len, int __prot,
            int __flags, int __fd, __off_t __offset) {
    return do_mmap(current->thread.mm, __addr, __len, __prot);
}
```

##### 5.6.1.1 实现get_unmapped_area函数

如上文所提到，若建议的虚拟地址区域 `[__addr, __addr+__len)`与已有的映射冲突，则需要另外寻找无冲突的虚拟地址区域，该过程由 `get_unmapped_area`函数完成：

```C
unsigned long get_unmapped_area(size_t length);
```

我们采用最简单的暴力搜索方法来寻找未映射的长度为 `length` (按页对齐) 的虚拟地址区域：从0地址开始向上以 `PAGE_SIZE`为单位遍历，直到遍历到连续 `length`长度内均无已有映射的地址区域，将该区域的首地址返回。

##### 5.6.1.2 实现do_mmap函数

我们可以把 `mmap`函数看作一个封装，实际工作的函数为

```c
void *do_mmap(struct mm_struct *mm, void *start, size_t length, int prot);
```

`do_mmap`函数主要完成了以下几个步骤：

- 检查建议的虚拟地址区域`[start, start+length)`是否与已有映射冲突，若冲突，则调用`get_unmapped_area`生成新的地址区域
- 为配合需求分页机制，需要创建一个`vm_area_struct`记录该虚拟地址区域的信息，并添加到`mm`的链表中
  - 调用`kmalloc`分配一个新的`vm_area_struct`
  - 设置`vm_area_struct`的起始和结束地址为`start`,`start + length`
  - 设置`vm_area_struct`的`vm_flags`为`prot`
  - 将该`vm_area_struct`插入到`mm`的链表中
- 返回地址区域的首地址

##### 5.6.1.3 实现mmap函数并添加到系统调用

- `mmap`实际为 `do_mmap`的封装，函数体中只需要将其对应参数传参给 `do_mmap`函数即可：

  ```c
  return do_mmap(current->mm, __addr, __len, __prot);
  ```
- 仿照lab5的做法，将 `mmap`系统调用添加到异常处理函数 `handler_s`中，系统调用号为222

#### 5.6.2 修改task_init函数代码，更改为需求分页机制

在之前的实验中，`task_init`函数为各个进程初始化时，已经为它们分配好了物理内存空间并进行了页表映射。本次实验开始改用需求分页机制，因此需要对 `task_init`函数代码进行改动

- 删除之前实验中为进程分配物理内存空间与页表映射的代码
- 调用`do_mmap`函数，建立用户进程的虚拟地址空间信息，包括两个区域
  - 代码区域，即用户进程的`.text`字段。该区域从虚拟地址0开始，权限为`PROT_READ | PROT_WRITE | PROT_EXEC`
  - 用户栈。该区域与lab5一样，范围为`[USER_END - PAGE_SIZE, USER_END)`，其中`USER_END = 0xffffffdf80000000`，权限为`PROT_READ | PROT_WRITE`

```c

    sscratch_top = kmalloc(PAGE_SIZE) + PAGE_SIZE;
    task[1]->state = TASK_RUNNING;
    #ifdef SJF
    task[1]->counter = rand();
    #endif
    #ifdef PRIORITY
    task[1]->counter = 8 - 1;
    #endif
    task[1]->priority = 5;
    task[1]->blocked = 0;
    task[1]->pid = 1;
    task[1]->thread.sp = USER_END;

    // initial_kernel_stack(sscratch_top, sscratch_top - 280);
    task[1]->thread.sepc = 0;
    task[1]->thread.ra = (unsigned long)task_first_ret;
    task[1]->thread.sscratch = sscratch_top;
    task[1]->thread.mm = kmalloc(sizeof(struct mm_struct));
    task[1]->thread.mm->pgtbl = (unsigned long)kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
    task[1]->thread.mm->vm_area_head = NULL;
    task[1]->thread.mm->pa_for_stack = 0;
    unsigned long pgtbl_va = (unsigned long)task[1]->thread.mm->pgtbl;
    initial_pgtbl(pgtbl_va);
    //建立用户进程的虚拟地址空间信息，包括两个区域
    //代码区域，即用户进程的`.text`字段。该区域从虚拟地址0开始，权限为`PROT_READ | PROT_WRITE | PROT_EXEC`
    do_mmap(task[1]->thread.mm, 0, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    //用户栈,范围为`[USER_END - PAGE_SIZE, USER_END)`，`USER_END = 0xffffffdf80000000`，权限为`PROT_READ | PROT_WRITE`
    do_mmap(task[1]->thread.mm, USER_END - PAGE_SIZE, PAGE_SIZE, PROT_READ | PROT_WRITE);
```

#### 5.6.3 实现munmap函数

`munmap`函数是 `mmap`函数的逆操作，用于解除 `mmap`映射的虚拟地址区域：

若操作成功，则返回0，否则返回-1

##### 5.6.3.1 实现free_page_tables函数

`free_page_tables`函数相当于lab4中实现的 `create_mapping`函数的逆操作.

```c
unsigned long free_table_dir(unsigned long *tblptr, unsigned long va, unsigned long right) {
    tblptr = (unsigned long)tblptr - 0x80000000ul + page_offset;
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            return free_table_dir(((((tblptr[tbl_index]) >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12), va, right - 9);
        } else {
            tblptr[tbl_index] ^= 1;
            return (tblptr[tbl_index] >> 10) << 12;
        }
    } else {
        return NULL;
    }
}

void free_page_tables(unsigned long pgtbl, unsigned long va, unsigned long sz, int free_frame) {
    //`pagetable`为页表基地址，可从`task_struct`中读取或直接从`satp`获取
    //`va`为需要解除映射的虚拟地址的起始地址，`n`为需要解除映射的页的数量
    unsigned long i = 0;
    for (i = 0; i < sz; i += 0x1000) {
        unsigned long pa = free_table_dir(pgtbl, va + i, 30);
        if (pa != NULL && free_frame) {
            kfree(pa + kmem_struct.virtual_offset);
        }
    }
}
```

- `pagetable`为页表基地址，可从`task_struct`中读取或直接从`satp`获取
- `va`为需要解除映射的虚拟地址的起始地址，`n`为需要解除映射的页的数量
- `free_frame`为布尔值，决定在解除映射时是否将物理页释放

##### 5.6.3.2 实现munmap函数

- 遍历`current->mm`中的`vm_area_struct`链表，找到虚拟内存区域`[start, start+length)`对应的链表节点，若找不到则返回-1
- 调用`free_page_tables`解除相应的页表映射并释放对应的物理页，`free_frame`参数设置为1
- 从`current->mm`中的`vm_area_struct`链表删除该区域对应的链表节点，并`kfree`释放该节点的内存空间
- 仿照lab5的做法，将`munmap`系统调用添加到异常处理函数`handler_s`中，系统调用号为215

```c
//是 `mmap`函数的逆操作，用于解除 `mmap`映射的虚拟地址区域：
int munmap(void *start, size_t length) {
    struct vm_area_struct *found = NULL;
    //找到虚拟内存区域`[start, start+length)`对应的链表节点
    if (current->thread.mm->vm_area_head == NULL) {
        return -1;
    } else {
        struct vm_area_struct *ptr;
        for (ptr = current->thread.mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
            if (ptr->vm_start == (unsigned long)start && ptr->vm_end == (unsigned long)start + length) {
                found = ptr;
                break;
            }
        }
    }

    if (found == NULL) {
        return -1;
    }
    //解除相应的页表映射并释放对应的物理页
    free_page_tables(current->thread.mm->pgtbl, start, length, 1);
    //从`current->mm`中的`vm_area_struct`链表删除该区域对应的链表节点，并`kfree`释放该节点的内存空间
    if (found->vm_prev == NULL && found->vm_next == NULL) {
        current->thread.mm->vm_area_head = NULL;
    } else if (found->vm_prev == NULL) {
        found->vm_next->vm_prev = NULL;
        current->thread.mm->vm_area_head = found->vm_next;
    } else if (found->vm_next == NULL) {
        found->vm_prev->vm_next = NULL;
    } else {
        found->vm_prev->vm_next = found->vm_next;
        found->vm_next->vm_prev = found->vm_prev;
    }
    kfree(found);
    return 0;
}
```

#### 5.6.4 实现mprotect函数

在Linux中，使用 `mprotect`函数可以用来修改一段指定内存区域（从 `void *__addr`开始, 长度为 `size_t __len`字节）的内存保护属性值 `int __prot`

```C
//修改一段指定内存区域（从 `void *__addr`开始, 长度为 `size_t __len`字节）的内存保护属性值 `int __prot`
int mprotect (void *__addr, size_t __len, int __prot) {
    struct vm_area_struct *found = NULL;
    if (current->thread.mm->vm_area_head == NULL) {
        return -1;
    } else {
        struct vm_area_struct *ptr;
        for (ptr = current->thread.mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
            if (ptr->vm_start == (unsigned long)__addr && ptr->vm_end == (unsigned long)__addr + __len) {
                found = ptr;
                break;
            }
        }
    }
    if (found == NULL) {
        return -1;
    }
    protect_page_tables(current->thread.mm->pgtbl, __addr, __len, __prot);
    found->vm_flags = __prot;
    found->vm_page_prot.pgprot = __prot << 1;
    return 0;
}
```

- `mprotect`中使用的`__prot`参数与`mmap`相同
- 遍历页表至第三级子页表，根据`__prot`参数重新设置页表中的权限位
- 仿照lab5的做法，将`mprotect`系统调用添加到异常处理函数`handler_s`中，系统调用号为226

### 5.7 实现 Page Fault 的检测与处理

+ 修改 `strap.c`文件, 添加对于Page Fault异常的检测

  RISC-V下的中断异常可以分为两类，一类是interrupt，另一类是exception，具体的类别信息可以通过解析scause[XLEN-1]获得。在前面的实验中，我们已经实现在interrupt中添加了时钟中断处理，在exception中添加了用户态程序的系统调用请求处理。在本次实验中，我们还将继续添加对于Page Fault的处理，即当Page Fault发生时，能够根据scause寄存器的值检测出该异常，并调用针对该类异常的处理函数 `do_page_fault`。

  当Page Fault发生时scause寄存器可能的取值声明如下：

  ```c
  # define CAUSE_FETCH_PAGE_FAULT    12
  # define CAUSE_LOAD_PAGE_FAULT     13
  # define CAUSE_STORE_PAGE_FAULT    15
  ```

  注：可在 `strap.c`中实现使得所有类型的Page Fault处理都最终能够调用到同一个 `do_page_fault`，然后再在该处理函数中继续判断不同Page Fault的类型，并针对该类Page Fault做进一步的处理。
+ 在 `fault.c`中实现 Page Fault 处理函数:  `do_page_fault`

  + 根据前文 `5.6节`中所描述的 `mmap`映射虚拟地址区域所采用的需求分页（Demand Paging）机制。等进程调度结束，首次切换到某一进程的时候，由于访问了未映射的虚拟内存地址，即会触发产生Page Fault异常。
  + `do_page_fault` 处理函数中具体实现的功能如下：
  + 1. 读取`csr STVAL`寄存器，获得访问出错的虚拟内存地址（`Bad Address`），并打印出该地址。
    2. 检查访问出错的虚拟内存地址（`Bad Address`）是否落在该进程所定义的某一`vm_area`中，即遍历当前进程的`vm_area_struct`链表，找到匹配的`vm_area`。若找不到合适的`vm_area`，则退出Page Fault处理函数，同时打印输出`Invalid vm area in page fault`。
    3. 根据`csr SCAUSE`判断Page Fault类型。
    4. 根据Page Fault类型，检查权限是否与当前`vm_area`相匹配。
       + 当发生`Page Fault caused by an instruction fetch `时，Page Fault映射的`vm_area`权限需要为可执行；
       + 当发生`Page Fault caused by a read`时，Page Fault映射的`vm_area`权限需要为可读；
       + 当发生`Page Fault caused by a write`时，Page Fault映射的`vm_area`权限需要为可写；
       + 若发生上述权限不匹配的情况，也需要退出Page Fault处理函数，同时打印输出`Invalid vm area in page fault`。
       + 只有匹配到合法的`vma_area`后，才可进行相应的物理内存分配以及页表的映射。
    5. 最后根据访问出错的`Bad Address`，调用Lab4中实现过的`create_mapping`实现新的页表映射关系。注意此时的Bad Address不一定恰好落在内存的4K对齐处，因此需要稍加处理，取得合适的虚拟内存和物理内存的映射地址以及大小。

```c
void do_page_fault() {
    
    unsigned long scause;
    __asm__ __volatile__ ("csrr %0, scause\n\t" : "=r" (scause));

    unsigned long sepc;
    __asm__ __volatile__ ("csrr %0, sepc\n\t" : "=r" (sepc));

    unsigned long stval;
    __asm__ __volatile__ ("csrr %0, stval\n\t" : "=r" (stval));
    //获得访问出错的虚拟内存地址（`Bad Address`），并打印出该地址。
    puts("[S] PAGE_FAULT: PID = ");
    puti(current->pid);
    puts("\n[S] PAGE_FAULT: scause: ");
    puti(scause);
    puts(", sepc: 0x");
    puti64(sepc);
    puts(", badaddr: 0x");
    puti64(stval);
    puts("\n");

    struct vm_area_struct *found = NULL;
    //遍历当前进程的`vm_area_struct`链表，找到匹配的`vm_area`。
    if (current->thread.mm->vm_area_head == NULL) {
        puts("Invalid vm area in page fault");
        return;
    } else {
        struct vm_area_struct *ptr;
        for (ptr = current->thread.mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
            if (ptr->vm_start <= stval && stval < ptr->vm_end) {
                found = ptr;
                break;
            }
        }
    }
    if (found == NULL) {
        puts("Invalid vm area in page fault\n");
        return;
    }
    //检查权限是否与当前`vm_area`相匹配。
    if (scause == 12 && (found->vm_flags & PROT_READ) == 0 && (found->vm_flags & PROT_EXEC) == 0 && (found->vm_flags & PROT_WRITE) == 0) {
        puts("Invalid vm area in page fault\n");
        return;
    } else if (scause == 13 && (found->vm_flags & (PROT_READ)) == 0) {
        puts("Invalid vm area in page fault\n");
        return;
    } else if (scause == 15 && (found->vm_flags & PROT_READ) == 0 && (found->vm_flags & PROT_WRITE) == 0) {
        puts("Invalid vm area in page fault\n");
        return;
    }
    //匹配到合法的`vma_area`后，才可进行相应的物理内存分配以及页表的映射。
    unsigned pa;
    unsigned long sz = (found->vm_end - found->vm_start + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    //注意需要特判fork⼦进程的栈⻚和进程的代码段，这时不需要⽤kmalloc来获取物理⻚，前者通过映射到记录的已复制的⼦进程物理⻚，后者映射到0x84000000 
    if (current->thread.mm->pa_for_stack != 0 && found->vm_end == USER_END) {
        pa = current->thread.mm->pa_for_stack;
        create_mapping(current->thread.mm->pgtbl, found->vm_start, current->thread.mm->pa_for_stack, sz, found->vm_page_prot.pgprot | (1 << 4));
    } else if (found->vm_start == 0) {
        pa = 0x84000000ul;
        create_mapping(current->thread.mm->pgtbl, found->vm_start, 0x84000000ul, sz, found->vm_page_prot.pgprot | (1 << 4));
    } else {
        pa = kmalloc(sz) - page_offset + 0x80000000ul;
        create_mapping(current->thread.mm->pgtbl, found->vm_start / PAGE_SIZE * PAGE_SIZE, pa, sz, found->vm_page_prot.pgprot | (1 << 4));
    }
    
    puts("[S] mapped PA: ");
    puti64(pa);
    puts(" to VA: ");
    puti64(found->vm_start);
    puts(" with size: ");
    puti64(sz);
    puts("\n");
}
```



### 5.8 实现 `fork`系统调用

#### 5.8.1 修改task_struct

修改task_struct，添加成员：

```c
uint64 *stack；
```

`stack`成员用于保存异常发生时的寄存器状态，会在fork函数中使用。可以在每次异常处理 `handler_s`触发时都将当前的寄存器状态保存到 `stack`成员中。

#### 5.8.2 实现fork函数

`fork`函数的函数原型如下：

```c
uint64 fork(void)
```

* 父进程`fork`成功时`返回：子进程的pid`，子进程`返回：0`。`fork`失败则父进程`返回：-1`。

`fork`具体的操作如下：

* 创建子进程，仿照lab5的方式，设置好`task_struct`中的`sp`,`sepc`,`sstatus`和`sscratch`成员。根据lab5设置的不同，`sscratch`成员可能设置为陷入异常处理的用户态sp或其他值。
* 为子进程`task_struct`中的`stack`成员分配空间，并将父进程的`stack`内容拷贝进去，注意为了让子进程的返回值为0，需要修改寄存器a0状态。
* 为子进程分配并拷贝父进程的`mm_struct`。
* 为子进程新建页表，为了适应page_fault，这里同样只映射内核的映射。
* 拷贝用户栈。我们需要分配一个页的空间并将父进程的用户栈即`[USER_END - PAGE_SIZE, USER_END)`拷贝进去，作为子进程的用户栈。子进程的用户栈起始地址需要保存到`task_struct`中，等待page_fault为其建立映射（5.8.5节）。
* 设置`task_struct`中的`ra`成员为5.8.3中的`forkret`函数。
* 返回子进程的`pid`，结束。
* 仿照lab5的做法，将`fork`系统调用添加到异常处理函数`handler_s`中，系统调用号为220。

```c

int fork() {
    ++task_num_top;
    task[task_num_top] = kmalloc(sizeof(struct task_struct));
    task[task_num_top]->state = TASK_RUNNING;
    unsigned long sscratch_top = kmalloc(PAGE_SIZE) + PAGE_SIZE;
    task[task_num_top]->state = TASK_RUNNING;
    #ifdef SJF
    task[task_num_top]->counter = rand(); 
    # endif
    #ifdef PRIORITY
    task[task_num_top]->counter = 8 - task_num_top;
    #endif
    // 
    task[task_num_top]->priority = 5;
    task[task_num_top]->blocked = 0;
    task[task_num_top]->pid = task_num_top;
    task[task_num_top]->thread.sp = USER_END;

    puts("[PID = ");
    puti(task[task_num_top]->pid);
    puts("] Process fork from [PID = ");
    puti(current->pid);
    puts("] Successfully! counter = ");
    puti(task[task_num_top]->counter);
    puts("\n");

    // initial_kernel_stack(sscratch_top, sscratch_top - 280);
    task[task_num_top]->thread.sepc = current->thread.sepc;
    task[task_num_top]->thread.ra = (unsigned long)forkret;
    task[task_num_top]->thread.sscratch = sscratch_top;
    task[task_num_top]->thread.sp = sscratch_top - 280;
    task[task_num_top]->thread.mm = kmalloc(sizeof(struct mm_struct));
    task[task_num_top]->thread.mm->pgtbl = (unsigned long)kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
    task[task_num_top]->thread.mm->vm_area_head = NULL;
    task[task_num_top]->thread.mm->pa_for_stack = 0;
    unsigned long pgtbl_va = (unsigned long)task[task_num_top]->thread.mm->pgtbl;
    initial_pgtbl(pgtbl_va);
    //子进程需要拷贝父进程 `task_struct`、`根页表`、`mm_struct`以及父进程的 `用户栈`等信息。

    if (current->thread.mm->vm_area_head != NULL) {
        struct vm_area_struct *tail = NULL;
        for (struct vm_area_struct *ptr = current->thread.mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
            struct vm_area_struct *t = kmalloc(sizeof(struct vm_area_struct));
            t->vm_start = ptr->vm_start;
            t->vm_end = ptr->vm_end;
            t->vm_flags = ptr->vm_flags;
            t->vm_page_prot = ptr->vm_page_prot;
            t->vm_mm = task[task_num_top]->thread.mm;
            t->vm_prev = tail;
            t->vm_next = NULL;
            if (tail == NULL) {
                task[task_num_top]->thread.mm->vm_area_head = t;
            } else {
                tail->vm_next = t;
            }
            tail = t;
        }
    }

    task[task_num_top]->thread.stack = task[task_num_top]->thread.sp;
    //为子进程`task_struct`中的`stack`成员分配空间，并将父进程的`stack`内容拷贝进去，注意为了让子进程的返回值为0，需要修改寄存器a0状态。
    for (int i = 0; i < 280 / 8; ++i) {
        task[task_num_top]->thread.stack[i] = current->thread.stack[i];
    }
    task[task_num_top]->thread.stack[9] = 0;
    char *new_stack = kmalloc(PAGE_SIZE);
    task[task_num_top]->thread.mm->pa_for_stack = (unsigned long)new_stack - page_offset + 0x80000000ul;
    for (int i = 0; i < PAGE_SIZE; ++i) {
        new_stack[i] = ((char*)(USER_END - PAGE_SIZE))[i];
    }
    
    return task_num_top;// `fork`成功时 `返回：子进程的pid`
}
```

#### 5.8.3 实现forkret函数

完成fork后，context switch到达子进程时该进程返回到的c语言函数，它的作用主要是调用5.8.4中的汇编函数 `ret_form_fork`使fork结束的子进程返回到用户模式。

在fork 里面把ra置为forkret `task[task_num_top]->thread.ra = (*unsigned* *long*)forkret;`

具体操作：

* 调用5.8.4中的汇编函数`ret_form_fork`返回到用户模式。

  ```c
  void forkret() {
      asm volatile ("csrrw x0, sscratch, %0" : : "r"(current->thread.sscratch));
      ret_from_fork(current->thread.stack);
  }
  ```

#### 5.8.4 实现ret_from_fork函数

这是一个汇编函数

```c
extern void ret_from_fork(uint64 *stack);
```

* 该函数主要是读取`stack`中的所有寄存器状态并返回到用户模式，调用时`stack`是当前`task_struct`中的成员。
* 读取寄存器时需要注意读取的顺序，以及lab5中加入的`sscratch`机制，保证系统正常运行。

```assembly
# ⼦进程退出handler_s时候的返回函数，主要为调⽤ret_from_fork
# 这⾥从传⼊的stack参数中恢复寄存器。
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
```

#### 5.8.5 修改 Page Fault 处理

在之前的Page Fault处理中，我们对用户栈Page Fault处理方法是自由分配一页作为用户栈并映射到 `[USER_END - PAGE_SIZE, USER_END)`的虚拟地址。但由fork创建的进程，它的用户栈已经拷贝完毕，因此Page Fault处理时直接为该页建立映射即可。

#### 5.8.6 修改task_init函数代码，使fork正常运行

我们新提供的user.bin中添加了两次 `fork`调用，为了使其正常运行，我们需要在 `task_init`函数中修改为仅初始化一个进程，其余三个进程均通过 `fork`创建。

### 5.9 编译及测试

本次实验的输出结果参考如下：

```
ZJU OS LAB 6             GROUP-XX
[S] Create New cache: name:slub-objectsize-8    	size: 0x00000008, align: 0x00000008
[S] Create New cache: name:slub-objectsize-16   	size: 0x00000010, align: 0x00000008
[S] Create New cache: name:slub-objectsize-32   	size: 0x00000020, align: 0x00000008
[S] Create New cache: name:slub-objectsize-64   	size: 0x00000040, align: 0x00000008
[S] Create New cache: name:slub-objectsize-128  	size: 0x00000080, align: 0x00000008
[S] Create New cache: name:slub-objectsize-256  	size: 0x00000100, align: 0x00000008
[S] Create New cache: name:slub-objectsize-512  	size: 0x00000200, align: 0x00000008
[S] Create New cache: name:slub-objectsize-1024 	size: 0x00000400, align: 0x00000008
[S] Create New cache: name:slub-objectsize-2048 	size: 0x00000800, align: 0x00000008
[S] Buddy allocate addr: ffffffff80023000
[S] Buddy allocate addr: ffffffff80098000
[S] Buddy allocate addr: ffffffff80099000
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff8008000000000008, partial_obj_count: 1
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff8008004000000008, partial_obj_count: 2
[S] New vm_area_struct: start 0000000000000000, end 00000000000000fd, prot [r:1,w:1,x:1]
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff8008008000000008, partial_obj_count: 3
[S] New vm_area_struct: start fffffffefffff000, end ffffffff00000000, prot [r:1,w:1,x:0]
[PID = 1] Process Create Successfully! counter = 3
[PID = 0] Context Calculation: counter = 1
[!] Switch from task 0[ffffffff80009050] to task 1[ffffffff80009148], prio: 5, counter: 3
[S] PAGE_FAULT: PID = 1
[S] PAGE_FAULT: scause: 12, sepc: 0x00000000, badaddr: 0x00000000
[S] mapped PA :0000000080008000 to VA :0000000000000000 with size :00000000000000fd
[S] PAGE_FAULT: PID = 1
[S] PAGE_FAULT: scause: 15, sepc: 0x000000b4, badaddr: 0xfffffff8
[S] Buddy allocate addr: ffffffff8009c000
[S] mapped PA :000000008009c000 to VA :fffffffefffff000 with size :0000000000001000
[S] Buddy allocate addr: ffffffff8009f000
[S] kmem_cache_alloc: name: slub-objectsize-256 
addr: ffffffff8008800000000008, partial_obj_count: 1
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff800800c000000008, partial_obj_count: 4
[S] Buddy allocate addr: ffffffff800a0000
[S] Buddy allocate addr: ffffffff800a1000
[PID = 2] Process fork from [PID = 1] Successfully! counter = 3
[S] Buddy allocate addr: ffffffff800a2000
[S] kmem_cache_alloc: name: slub-objectsize-256 
addr: ffffffff8008810000000008, partial_obj_count: 2
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff8008010000000008, partial_obj_count: 5
[S] Buddy allocate addr: ffffffff800a3000
[S] Buddy allocate addr: ffffffff800a4000
[PID = 3] Process fork from [PID = 1] Successfully! counter = 3
pid: 1
[PID = 1] Context Calculation: counter = 3
[PID = 1] Context Calculation: counter = 2
[PID = 1] Context Calculation: counter = 1
[!] Switch from task 1[ffffffff80009148] to task 3[ffffffff80009338], prio: 5, counter: 3
[S] PAGE_FAULT: PID = 3
[S] PAGE_FAULT: scause: 12, sepc: 0x00000094, badaddr: 0x00000094
[S] mapped PA :0000000080008000 to VA :0000000000000094 with size :00000000000000fd
[S] PAGE_FAULT: PID = 3
[S] PAGE_FAULT: scause: 15, sepc: 0x00000098, badaddr: 0xffffffc8
[S] mapped PA :00000000800a4000 to VA :fffffffefffff000 with size :0000000000001000
pid: 3
[PID = 3] Context Calculation: counter = 3
[PID = 3] Context Calculation: counter = 2
[PID = 3] Context Calculation: counter = 1
[!] Switch from task 3[ffffffff80009338] to task 2[ffffffff80009240], prio: 5, counter: 3
[S] PAGE_FAULT: PID = 2
[S] PAGE_FAULT: scause: 12, sepc: 0x00000094, badaddr: 0x00000094
[S] mapped PA :0000000080008000 to VA :0000000000000094 with size :00000000000000fd
[S] PAGE_FAULT: PID = 2
[S] PAGE_FAULT: scause: 15, sepc: 0x00000098, badaddr: 0xffffffc8
[S] mapped PA :00000000800a1000 to VA :fffffffefffff000 with size :0000000000001000
[S] Buddy allocate addr: ffffffff800ad000
[S] kmem_cache_alloc: name: slub-objectsize-256 
addr: ffffffff8008820000000008, partial_obj_count: 3
[S] kmem_cache_alloc: name: slub-objectsize-64  
addr: ffffffff8008014000000008, partial_obj_count: 6
[S] Buddy allocate addr: ffffffff800ae000
[S] Buddy allocate addr: ffffffff800af000
[PID = 4] Process fork from [PID = 2] Successfully! counter = 3
pid: 2
[PID = 2] Context Calculation: counter = 3
[PID = 2] Context Calculation: counter = 2
[PID = 2] Context Calculation: counter = 1
[!] Switch from task 2[ffffffff80009240] to task 4[ffffffff80009430], prio: 5, counter: 3
[S] PAGE_FAULT: PID = 4
[S] PAGE_FAULT: scause: 12, sepc: 0x00000094, badaddr: 0x00000094
[S] mapped PA :0000000080008000 to VA :0000000000000094 with size :00000000000000fd
[S] PAGE_FAULT: PID = 4
[S] PAGE_FAULT: scause: 15, sepc: 0x00000098, badaddr: 0xffffffc8
[S] mapped PA :00000000800af000 to VA :fffffffefffff000 with size :0000000000001000
pid: 4
[PID = 4] Context Calculation: counter = 3
[PID = 4] Context Calculation: counter = 2
[PID = 4] Context Calculation: counter = 1
[*] Reset task 1 counter 5
[*] Reset task 2 counter 5
[*] Reset task 3 counter 5
[!] Switch from task 4[ffffffff80009430] to task 3[ffffffff80009338], prio: 5, counter: 5
pid: 3
[PID = 3] Context Calculation: counter = 5
[PID = 3] Context Calculation: counter = 4
[PID = 3] Context Calculation: counter = 3
pid: 3
[PID = 3] Context Calculation: counter = 2
[PID = 3] Context Calculation: counter = 1
[!] Switch from task 3[ffffffff80009338] to task 2[ffffffff80009240], prio: 5, counter: 5
pid: 2
[PID = 2] Context Calculation: counter = 5
[PID = 2] Context Calculation: counter = 4
[PID = 2] Context Calculation: counter = 3
pid: 2
[PID = 2] Context Calculation: counter = 2
[PID = 2] Context Calculation: counter = 1
[!] Switch from task 2[ffffffff80009240] to task 1[ffffffff80009148], prio: 5, counter: 5
pid: 1
[PID = 1] Context Calculation: counter = 5
[PID = 1] Context Calculation: counter = 4
[PID = 1] Context Calculation: counter = 3
pid: 1
[PID = 1] Context Calculation: counter = 2
[PID = 1] Context Calculation: counter = 1
[*] Reset task 2 counter 5
[*] Reset task 3 counter 5
[*] Reset task 4 counter 5
[!] Switch from task 1[ffffffff80009148] to task 4[ffffffff80009430], prio: 5, counter: 5
pid: 4
...
```

![image-20211220105448612](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211220105448612.png)

![image-20211220105508329](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211220105508329.png)

![image-20211220105532350](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211220105532350.png)

![image-20211220105550826](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211220105550826.png)

![image-20211220123750216](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211220123750216.png)

### 6. 实验任务与要求

  请仔细阅读背景知识，完成 5.1 至 5.9的内容，撰写实验报告, 需提交实验报告以及整个工程的压缩包。

- 实验报告：
  * 各实验步骤截图以及结果分析
  * 回答思考题
  * 实验结束后心得体会
  * 对实验指导的建议（可选） 
- 工程文件
  * 所有source code（确保make clean）
  * 将lab6_319010XXXX目录压缩成zip格式
- 最终提交
  * 将报告和工程压缩包提交至学在浙大

#### 心得体会

我进一步学习了内存分配. 用buddy system 管理内存, 二叉树的实际应用, 怎么用二叉树来分配, 释放. 在小内存分配的时候, 我们自己实现了对象缓存的slab. 深入学习了VMA, 一个链表连接一个个虚拟地址块. 

释放内存, 在链表中释放VMA, 解除相应的页表映射并释放对应的物理页. 用代码实现fork后共享代码段.

create_mapping的传⼊地址是虚拟地址还是物理地址?

page table 内容是物理地址，kmalloc返回的地址均设为虚拟地址。因为我们kmalloc返回的虚拟地址是内核地址，所以可以做简单的减法变换得到物理地址。在buddy.c中设置page_offset = 0x80000000ul。

##### 对实验的建议

lab6其实很多还是很重要的, 比如mmap,  slab, fork的具体实现. 感觉作为bonus有点可惜, 但是量实在有点大. 作为一次实验量有点太大了, 是否可以少实现几个系统调用.  或者分成两个实验. 

fork提供的提示有些少，好多要自己摸索。可以画⼀张图说明⼀下fork父进程和子进程调用时的栈信息。

不教makefile, 但是要写多个文件夹的makefile, 比较头疼. 这不属于课程范围, 但是会耗费学生较多时间精力去查询语法命令. 

