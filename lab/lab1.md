# Lab 1: RV64 内核引导

## 1 实验目的

学习RISC-V相关知识，了解OpenSBI平台，实现sbi调用函数，封装打印函数，并利用Makefile来完成对整个工程的管理。	

为降低实验难度，我们选择使用OpenSBI作为BIOS来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印等的操作。

## 2 实验内容及要求

* 阅读RISC-V中文手册，学习RISC-V相关知识
* 学习Makefile编写规则，补充Makefile文件使得项目成功运行
* 了解OpenSBI的运行原理，编写代码通过sbi调用实现字符串的打印

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，跟随实验步骤完成实验，以截图的方式记录命令行的输入与输出，注意进行适当的代码注释。如有需要，请对每一步的命令以及结果进行必要的解释。

在最终提交的实验报告中，**请自行删除附录部分内容**，并命名为“**学号_姓名\_lab1.pdf**"，代码文件请根据给出的结构整理好后打包命名为“**学号_姓名\_lab1**"，分开上传至学在浙大平台。
## 3 实验步骤

### 3.1 搭建实验环境（10%）

按照以下方法创建新的容器，并建立volume映射([参考资料](https://kebingzao.com/2019/02/25/docker-volume/))，在本地编写代码，并在容器内进行编译检查。未来的实验中同样需要用该方法搭建相应的实验环境，但不再作具体指导，请理解每一步的命令并自行更新相关内容。**注意及时关闭运行中的容器**。

#### 1.创建容器并建立映射关系

```shell
### 首先新建自己本地的工作目录(比如lab1)并进入
$ mkdir lab1
$ cd lab1
$ pwd
~/.../lab1

### 查看docker已有镜像(与lab0同一个image)
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
oslab               2020                678605140682        46 hours ago        2.89GB

### 创建新的容器，同时建立volume映射
$ docker run -it -v `pwd`:/home/oslab/lab1 -u oslab -w /home/oslab 6786 /bin/bash
oslab@3c1da3906541:~$ 
```

**【请将该步骤中命令的输入及运行结果截图附在此处，接下来每一步同理。】**

前面的输入都一样

![image-20211001101624137](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211001101624137.png)

```bash
 docker run -it -v `pwd`:/home/oslab/lab1 -u oslab -w /home/oslab fa20 /bin/bash # 这个好像没有正确mount
  docker run -it -v /c/Users/12638/Desktop/operatingSystem/lab/lab1:/home/oslab/lab1 -u oslab -w /home/oslab fa20 /bin/bash
```

就是说user name是 oslab,  工作目录是/home/oslab , 把宿主机映射到容器的/home/oslab/lab1目录 

![image-20211001102641319](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211001102641319.png)

之后用exec 进入容器而不是重新run

```
 docker exec -it  -u oslab -w /home/oslab musing_lovelace /bin/bash
```

![image-20211001112315031](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211001112315031.png)

有这个提示应该是mount成功了. 之前错误挂载的时候touch 会显示错误: 没有权限. 

#### 2.测试映射关系

为测试映射关系是否成功，再打开一个终端窗口，并进入`lab1`目录下

```shell
###Terminal2
$ touch testfile
$ ls
testfile
```

在第一个终端窗口，即容器中确认是否挂载成功。确认映射关系建立成功后，可以在本地`lab1`目录下编写实验代码，文件将映射到容器中，因此可以直接在容器中进行实验。

```shell
###Terminal1
### 在docker中确认是否挂载成功
oslab@3c1da3906541:~$ pwd
/home/oslab
oslab@3c1da3906541:~$ cd lab1
oslab@3c1da3906541:~$ ls
testfile
```
![image-20211001112458139](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211001112458139.png)


### 3.2 了解项目框架，编写MakeFile（20%）

#### 1.编写Makefile文件

```
.
├── arch
│   └── riscv
│       ├── boot
│       ├── include
│       │   ├── print.h
│       │   ├── sbi.h
│       │   └── test.h
│       ├── kernel
│       │   ├── head.S
│       │   ├── main.c(需修改数字为学号)
│       │   ├── Makefile
│       │   ├── sbi.c（需通过内联汇编实现sbi调用）
│       │   ├── test.c
│       │   └── vmlinux.lds
│       ├── libs
│       │   ├── MAKEFILE（需编写字符串打印函数及数字打印函数）
│       │   └── print.c
│       └── Makefile
├── cp.bat
├── include
│   └── defs.h
└── Makefile
```

下载相关代码移动至`lab1`文件夹中。

项目代码结构由上图所示，通过最外层的Makefile中的"make run"命令进行编译与运行。请参考【附录A.Makefile介绍】中的基础知识，了解项目各结构中Makefile每一行命令的作用，编写`./arch/riscv/libs/Makefile`文件使得整个项目能够顺利编译，并将其填写至下述代码块中。其他步骤中的代码编写同理。

```makefile
#lab1/arch/riscv/libs/Makefile 
# YOUR MAKEFILE CODE
C_SRC       = $(sort $(wildcard *.c))
OBJ		    = $(patsubst %.c,%.o,$(C_SRC))

all : $(OBJ)

%.o:%.c
	${CC}  ${CFLAG}  -c $<
	
clean:
	$(shell rm *.o 2>/dev/null)
```

#### 2.解释Makefile命令

**请解释`lab1/Makefile`中的下列命令：**

```makefile
${LD} -T arch/riscv/kernel/vmlinux.lds arch/riscv/kernel/*.o arch/riscv/libs/*.o -o vmlinux
```

含义：`${LD}`被展开为`riscv64-unknown-elf-ld`，即链接命令。`-T arch/riscv/kernel/vmlinux.lds`选项表示使用`vmlinux.lds`作为链接器脚本。`-o vmlinux`选项指定输出文件的名称为`vmlinux`，整行命令的意思是将`arch/riscv/kernel/*.o`和`arch/riscv/libs/*.o`根据`vmlinux.lds`链接形成`vmlinux`。

```shell
${OBJCOPY} -O binary vmlinux arch/riscv/boot/Image
```

含义：

在Linux Kernel开发中，为了对vmlinux进行精简，通常使用objcopy丢弃调试信息与符号表并生成二进制文件，. `${OBJCOPY}`被展开为`riscv64-unknown-elf-objcopy`，复制命令。`-O binary vmlinux `选项表示 把vimlinux elf文件转换为二进制bin文件.这就是Image 放在boot文件夹下

```makefile
${MAKE} -C arch/riscv all
```

含义：
`${MAKE}`的含义可以参见[这里](https://stackoverflow.com/questions/38978627/what-is-the-variable-make-in-a-makefile)

递归地调用make, 在arch/riscv文件夹执行make, 而且也会带有原来的参数, 比如 -t , -q , -n .

all 就是目标,在里面也就是obj

### 3.3 学习RISC-V相关知识及特权架构（10%）

后续实验中将持续使用RISC-V指令集相关的内容，请参考【附录B.RISC-V指令集】了解相关知识，**下载并认真阅读[RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)，**掌握基础知识、基本命令及特权架构相关内容。

#### 1.基础命令掌握情况

请填写命令的含义。

```assembly
#1.加载立即数，t0=0x40000
li t0,0x40000

#2. 对于t0 中每一个为1 的位, 把控制状态寄存器satp的对应位置位.
csrw satp, t0

#3.  t0 = t0 -t1
sub t0,t0,t1

#4. 64位数据传输, 存储双字到x1
sd x1, 8(sp)

#5.la是risc-v指令集中的一条伪指令。伪指令，一般会被汇编器翻译成一条或者多条等价的实际指令,load address是一条地址加载指令. 把内存地址加载到sp寄存器, stack_top是一个地址标记. 
la sp,stack_top
```

### 3.3 通过OpenSBI接口实现字符串打印函数（60%）

#### 1.程序执行流介绍

对于本次实验，我们选择使用OpenSBI作为bios来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印等的操作。

请参考【附录B.OpenSBI介绍】了解OpenSBI平台的功能及启动方式，参考【附录D. Linux Basic】了解`vmlinux.lds`、`vmlinux`的作用，理解执行`make run`命令时程序的执行过程。

```shell
#make run 实际为执行lab1/Makefile中的该行命令
@qemu-system-riscv64 -nographic --machine virt -bios default -device loader,file=vmlinux,addr=0x80200000 -D log
```

QEMU模拟器完成从ZSBL到OpenSBI阶段的工作，使用-bios default选项将OpenSBI代码加载到0x80000000起始处。OpenSBI初始化完成后，跳转到0x80200000处。我们将`vmlinux`程序这一RISC-V 64-bit架构程序加载至0x80200000处运行。

`vmlinux.lds`链接脚本指定了程序的内存布局，最先加载的`.text.init`段代码为`head.S`文件的内容，顺序执行调用`main()`函数。`main()`函数调用了两个打印函数，通过`sbi_call()`向OpenSBI发起调用，完成字符的打印。

#### 2.编写sbi_call()函数（20%）

当系统处于m模式时，对指定地址进行写操作便可实现字符的输出。但我们编写的内核运行在s模式，需要使用OpenSBI提供的接口，让运行在m模式的OpenSBI帮我们实现输出。此时，运行在s模式的内核通过 `ecall` 发起sbi 调用请求，RISC-V CPU 会从 s 态跳转到 m 态的 OpenSBI 固件。

执行 `ecall` 前需要指定 sbi调用的编号，传递参数。一般而言，`a7(x17)` 为 SBI 调用编号，`a0(x10)`、`a1(x11)` 和 `a2(x12)` 寄存器为sbi调用参数。**其中`a7`是寄存器的ABI Name，在RISC-V命令中需要使用`x17`进行标识，详见RISC-V中文手册中寄存器-接口名称相关的对照表。**

我们需要编写内联汇编语句以使用OpenSBI接口，给出函数定义如下：

```C
typedef unsigned long long uint64_t;
uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2)
```

在该函数中，我们需要完成以下内容：

* 将sbi_type放到寄存器a7中，将arg0-arg2放入a0-a2中。

* 使用ecall指令发起sbi调用请求。

* OpenSBI的返回结果会放到a0中，我们需要将其取出，作为sbi_call的返回值。

请参考【附录C.内联汇编】相关知识，以内联汇编形式实现`lab1/arch/riscv/kernel/sbi.c`中的`sbi_call()`函数。

```c
#lab1/arch/riscv/kernel/sbi.c
uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret_val;
    __asm__ volatile ( 
        "mv x17, %[type]\n"
        "mv x10,%[src0]\n"
        "mv x11,%[src1]\n"
        "mv x12,%[src2]\n"
        "ecall\n"
        "mv %[dest],x10"
        : [dest] "=r" (ret_val) 
        : [type] "r" (sbi_type), [src0] "r" (arg0), [src1] "r" (arg1), [src2] "r" (arg2)
        : 
   );
    return ret_val;
}
```

向`sbi_call()`传入不同的`sbi_type`调用不同功能的OpenSBI接口，其对应关系如下表所示，其中每个函数的具体使用方法可参考[OpenSBI 文档](https://github.com/riscv/riscv-sbi-doc/blob/master/riscv-sbi.adoc#function-listing-1)。 为了利用OpenSBI接口打印字符，我们需要向`sbi_call()`函数传入`sbi_type=1`以调用`sbi_console_putchar(int ch)`函数，第一个参数`arg0`需要传入待打印字符的ASCII码，第二、三个参数的值可直接设为0。测试时，调用`sbi_call(1,0x30,0,0)`将会输出字符'0'。其中1代表输出，0x30代表'0'的ascii值，arg1-2不需要输入，可以用0填充。

<img src="https://mianbaoban-assets.oss-cn-shenzhen.aliyuncs.com/2020/12/RfMJBr.png" style="zoom:50%;" />

#### 3.编写字符串打印函数（40%）

需要在`./arch/riscv/libs/print.c`文件中通过调用`sbi_call()`实现字符串打印函数`int puts(char* str)`及数字打印函数`int put_num(uint64_t n)`，后者可将数字转换为字符串后调用前者执行。

```c
#./arch/riscv/libs/print.c
#include "defs.h"
extern sbi_call();
int puts(char* str){
	int i = 0 ;
	while( str[i] != '\0' ){
		sbi_call(1, str[i],0,0 );
		i += 1;
	}
    return 0;
}

int put_num(uint64_t n){
	//convert  unsigned long long uint64_t to string
	char * str ;
	int i = 0;
	int k = 0; 
	char temp;//临时变量，交换两个值时用到
	char index[] = "0123456789";
	uint64_t unum = n ;
	unsigned int radix = 10 ;
	str[i++] = index[unum % radix];
	unum /= radix;
	while(unum > 0){
		str[i++] = index[unum % radix];
		unum = unum/  radix;
	}
    for(int j = k;j <= (i-1)/2; j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
	str[i] = '\0';
	puts(str) ;
	return 0;
}
```

### 3.4 编译及测试

在`lab1/arch/riscv/kernel/main.c`中将`21922192`修改为你的学号，在项目最外层输入`make run`命令调用Makefile文件完成整个工程的编译及执行。

**如果编译失败，及时使用`make clean`命令清理文件后再重新编译。**

如果程序能够正常执行并打印出相应的字符串及你的学号，则实验成功。预期的实验结果如下，请附上相应的命令行及实验结果截图。

```shell
oslab@3c1da3906541:~/lab1$ make run
Hello RISC-V!
21922192
```

![image-20211018151524013](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211018151524013.png)

## 4 讨论和心得

请在此处填写实验过程中遇到的问题及相应的解决方式。

由于本实验为新实验，可能存在不足之处，欢迎同学们对本实验提出建议。

问题1:  `docker run -it -v C:\Users\12638\Desktop\operating system\lab\lab1:/home/oslab/lab1 -u oslab -w /home/oslab  fa20 /bin/bash`  一开始是这么输入的, 但是出错 docker: invalid reference format. 看了一下问题应该是C:后面就直接匹配了映射所以出错. 原来 `pwd`可以直接自动填入目前的路径, 非常好用, 感谢实验让我学会了很多命令的输入方法. 

问题2 : touch: cannot touch 'testlfile': Permission denied

ls -al 看了一下好像oslab用户没有写权限, 那就sudo chmod 777 lab1 但是还有密码.密码不是oslab

只好开个root 看看`cat /etc/shadow`, 密码非常长,`passwd oslab`修改成123. 

![image-20211001105131302](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211001105131302.png)

但是, 好像再次启动run密码又不一样? 应该是停止容器后进入而不能再run  

再密码改成o. run一次之后应该用 docker exec , 因为run 会重新开一个container.

问题3 : 但是没有映射上去. 创建了文件不会在本地出现. 

docker: Error response from daemon: c\Users\12638\Desktop\operatingSystem\lab\lab1\%!(EXTRA string=is not a valid Windows path).

用pwd会出错,  我用绝对路径来mount

问题4:联合汇编, 输出操作数和输入操作数看不懂了. 不知道怎么用的. 于是阅读例子

```c
int main(void) {
    int sum;
    int add1 = 100 ;
    int add2 = 200;
    __asm__ volatile (
    	#汇编指令
        "add %[dest],%[src1],%[src2]"
        #输出操作数
        : [dest] "=r" (sum) // 把C程序变量和add的操作数绑定
        #输入操作数
        : [src1] "r" (add1), [src2] "r" (add2)
        #可能影响的寄存器或存储器
        : "memory"
    );
    return ret_val;
}
```

怎么使用ecall指令发起sbi调用请求?

手册没说清楚, 就说通过引发环境调用异常来请求执行环境.

问题5 :print.c中用了 sprintf, 那么makefile中怎么引入它的头文件呢?makefile的 all又应该怎么写呢?  按道理应该和kernel的makefile差不多.为什么要把 每个.S  .c 替换为 .o? 

CFLAG是什么? cflag就是可以加一堆参数在前面, 在父makefile中定义了.

问题6:error: 'type' undeclared (first use in this function)

![image-20211018124254019](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211018124254019.png)

改成*sbi_type* ,然后make clean

在clean中也可以clean:  cat print.c

问题7 : make run 后一直打印乱码

sbi_call(1, *str*,0,0 ); 这里不能直接传入指针, 要一个个读入char 然后输出. 

可以用上节课的gdb试一试单步调试. 

![image-20211018135559259](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211018135559259.png)

问题8: 不断打印hello riscv

![image-20211018143335803](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20211018143335803.png)

gdb看, put_num打印执行了一部分又跳转到上面的puts.

为什么`str[i++] = index[unum % radix];` 之后不执行`unum /= radix;` 而是跳转到 Breakpoint 1, _start () at head.S:8
8               la sp,stack_top

我把内联汇编最后的"memory"去掉就好了 , 不去掉也可以, 不知道为啥就好了. 

## 附录

### A. Makefile介绍

Makefile是一种实现关系整个工程编译规则的文件，通过自动化编译极大提高了软件开发的效率。一个工程中源文件，按类型，功能，模块分别放在若干个目录中，Makefile定义了一系列规则来指定哪些文件需要先编译，哪些文件需要后编译，哪些文件需要重新编译，甚至可以执行操作系统的命令。

以C为例，源文件首先会生成中间目标文件，再由中间目标文件生成执行文件。在编译时，编译器只检测程序语法和函数、变量是否被声明。如果函数未被声明，编译器会给出一个警告，但可以生成Object File。而在链接程序时，链接器会在所有的Object File中找寻函数的实现，如果找不到，那到就会报告链接错误码。

Lab0中我们已经使用了make工具利用Makefile文件来管理整个工程。请阅读[Makefile介绍 ](https://seisman.github.io/how-to-write-makefile/introduction.html)，根据工程文件夹里Makefile的代码与注释来掌握一些基本的使用技巧。

### B. RISC-V指令集

请**下载并认真阅读[RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)，**掌握基础知识、基本命令及特权架构相关内容。

实验中涉及了特权相关的重要寄存器，在中文手册中进行了简要介绍，其具体布局以及详细介绍可以参考[The RISC-V Instruction Set Manual Volume II: Privileged Architecture Version 1.9.1](https://riscv.org/wp-content/uploads/2016/11/riscv-privileged-v1.9.1.pdf)。

#### RISC-V汇编指令

常用的汇编指令包括`la`、`li`、`j` 、`ld`等，可以自行在 [RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)附录中了解其用法。

RISC-V指令集中有一类**特殊寄存器CSRs(Control and Status Registers)**，这类寄存器存储了CPU的相关信息，只有特定的控制状态寄存器指令 (csrrc、csrrs、csrrw、csrrci、csrrsi、csrrwi等)才能够读写CSRs。例如，保存`sepc`的值至内存时需要先使用相应的CSR指令将其读入寄存器，再通过寄存器保存该值，写入sepc时同理。

```asm
csrr t0, sepc
sd t0, 0(sp)
```

#### RISC-V特权模式

RISC-V有三个特权模式：U（user）模式、S（supervisor）模式和M（machine）模式。它通过设置不同的特权级别模式来管理系统资源的使用。其中M模式是最高级别，该模式下的操作被认为是安全可信的，主要为对硬件的操作；U模式是最低级别，该模式主要执行用户程序，操作系统中对应于用户态；S模式介于M模式和U模式之间，操作系统中对应于内核态，当用户需要内核资源时，向内核申请，并切换到内核态进行处理。

本实验主要在S模式运行，通过调用运行在M模式的OpenSBI提供的接口操纵硬件。

| Level | Encoding | Name             | Abbreviation |
| ----- | -------- | ---------------- | ------------ |
| 0     | 00       | User/Application | U            |
| 1     | 01       | Supervisor       | S            |
| 2     | 10       | Reserved         |              |
| 3     | 11       | Machine          | M            |

### B. OpenSBI介绍

SBI (Supervisor Binary Interface)是 S-Mode 的 kernel 和 M-Mode 执行环境之间的标准接口，而OpenSBI项目的目标是为在M模式下执行的平台特定固件提供RISC-V SBI规范的开源参考实现。为了使操作系统内核可以适配不同硬件，OpenSBI提出了一系列规范对m-mode下的硬件进行了抽象，运行在s-mode下的内核可以按照标准对这些硬件进行操作。

**为降低实验难度，我们将选择OpenSBI作为bios来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印的操作。**

![](https://raw.githubusercontent.com/riscv/riscv-sbi-doc/master/riscv-sbi-intro1.png)

#### RISC-V启动过程

![启动过程](https://mianbaoban-assets.oss-cn-shenzhen.aliyuncs.com/2020/12/eU3yIz.png) 
上图是RISC-V架构计算机的启动过程：

- ZSBL(Zeroth Stage Boot Loader)：片上ROM程序，烧录在硬件上，是芯片上电后最先运行的代码。它的作用是加载FSBL到指定位置并运行。
- FSBL(First Stage Boot Loader ）：启动PLLs和初始化DDR内存，对硬件进行初始化，加载下一阶段的bootloader。
- OpenSBI：运行在m模式下的一套软件，提供接口给操作系统内核调用，以操作硬件，实现字符输出及时钟设定等工作。OpenSBI就是一个开源的RISC-V虚拟化二进制接口的通用的规范。
- Bootloader：OpenSBI初始化结束后会通过mret指令将系统特权级切换到s模式，并跳转到操作系统内核的初始化代码。这一阶段，将会完成中断地址设置等一系列操作。之后便进入了操作系统。

更多内容可参考[An Introduction to RISC-V Boot Flow](https://riscv.org/wp-content/uploads/2019/12/Summit_bootflow.pdf)。

为简化实验，从ZSBL到OpenSBI运行这一阶段的工作已通过QEMU模拟器完成。运行QEMU时，我们使用-bios default选项将OpenSBI代码加载到0x80000000起始处。OpenSBI初始化完成后，会跳转到0x80200000处。因此，我们所编译的代码需要放到0x80200000处。


### C. 内联汇编

我们在用c语言写代码的时候会遇到直接对寄存器进行操作的情况，这时候就需要用的内联汇编了。内联汇编的详细使用方法可参考[**GCC内联汇编**](https://mp.weixin.qq.com/s/Ln4qBYvSsgRvdiK1IJqI6Q)。

在此给出一个简单示例。其中，三个`:`将汇编部分分成了四部分。

第一部分是汇编指令，指令末尾需要添加'\n'。指令中以`%[name]`形式与输入输出操作数中的同名操作符`[name]`绑定，也可以通过`%数字`的方式进行隐含指定。假设输出操作数列表中有1个操作数，“输入操作数”列表中有2个操作数，则汇编指令中`%0`表示第一个输出操作数，`%1`表示第一个输入操作数，`%2`表示第二个输入操作数。

第二部分是输出操作数，第三部分是输入操作数。输入或者输出操作符遵循`[name] "CONSTRAINT" (variable)`格式，由三部分组成：

* `[name]`：符号名用于同汇编指令中的操作数通过同名字符绑定。

* `"constraint"`：限制字符串，用于约束此操作数变量的属性。字母`r`代表使用编译器自动分配的寄存器来存储该操作数变量，字母`m`代表使用内存地址来存储该操作数变量，字母`i`代表立即数。

  对于输出操作数而言，等号“=”代表输出变量用作输出，原来的值会被新值替换；加号“+”代表输出变量不仅作为输出，还作为输入。此约束不适用于输入操作数。

* `(variable)`：C/C++变量名或者表达式。

示例中，输出操作符`[ret_val] "=r" (ret_val)`代表将汇编指令中`%[ret_val]`的值更新到变量ret_val中，输入操作符`[type] "r" (type)`代表着将()中的变量`type`放入寄存器中。

第四部分是可能影响的寄存器或存储器，用于告知编译器当前内联汇编语句可能会对某些寄存器或内存进行修改，使得编译器在优化时将其因素考虑进去。

```asm
unsigned long long s_example(unsigned long long type,unsigned long long arg0) {
    unsigned long long ret_val;
    __asm__ volatile (
    	#汇编指令
        "mv x10, %[type]\n"
        "mv x11,%[arg0]\n"
        "mv %[ret_val], x12"
        #输出操作数
        : [ret_val] "=r" (ret_val)
        #输入操作数
        : [type] "r" (type), [arg0] "r" (arg0)
        #可能影响的寄存器或存储器
        : "memory"
    );
    return ret_val;
}
```

示例二定义了一个宏，其中`%0`代表着输入输出部分的第一个符号，即`val`。其中`#reg`是c语言的一个特殊宏定义语法，相当于将`reg`进行宏替换并用双引号包裹起来。例如`write_csr(sstatus,val)`宏展开会得到：
`({asm volatile ("csrw " "sstatus" ", %0" :: "r"(val)); })`

```C
#define write_csr(reg, val) ({
    asm volatile ("csrw " #reg ", %0" :: "r"(val)); })
```

### D. Linux Basic

#### vmlinux

vmlinux通常指Linux Kernel编译出的可执行文件（Executable and Linkable Format， ELF），特点是未压缩的，带调试信息和符号表的。在本实验中，vmlinux通常指将你的代码进行编译，链接后生成的可供QEMU运行的RISC-V 64-bit架构程序。如果对vmlinux使用**file**命令，你将看到如下信息：

```shell
$ file vmlinux 
vmlinux: ELF 64-bit LSB executable, UCB RISC-V, version 1 (SYSV), statically linked, not stripped
```

#### System.map

System.map是内核符号表（Kernel Symbol Table）文件，是存储了所有内核符号及其地址的一个列表。使用System.map可以方便地读出函数或变量的地址，为Debug提供了方便。“符号”通常指的是函数名，全局变量名等等。使用`nm vmlinux`命令即可打印vmlinux的符号表，符号表的样例如下：

```asm
0000000000000800 A __vdso_rt_sigreturn
ffffffe000000000 T __init_begin
ffffffe000000000 T _sinittext
ffffffe000000000 T _start
ffffffe000000040 T _start_kernel
ffffffe000000076 t clear_bss
ffffffe000000080 t clear_bss_done
ffffffe0000000c0 t relocate
ffffffe00000017c t set_reset_devices
ffffffe000000190 t debug_kernel
```

#### vmlinux.lds

GNU ld即链接器，用于将\*.o文件（和库文件）链接成可执行文件。在操作系统开发中，为了指定程序的内存布局，ld使用链接脚本（Linker Script）来控制，在Linux Kernel中链接脚本被命名为vmlinux.lds。更多关于ld的介绍可以使用`man ld`命令。

下面给出一个vmlinux.lds的例子：

```asm
/* 目标架构 */
OUTPUT_ARCH( "riscv" )
/* 程序入口 */
ENTRY( _start )
/* 程序起始地址 */
BASE_ADDR = 0x80000000;
SECTIONS
{
  /* . 代表当前地址 */
  . = BASE_ADDR;
  /* code 段 */
  .text : { *(.text) }
  /* data 段 */
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  . += 0x8000;
  /* 栈顶 */
  stack_top = .;
  /* 程序结束地址 */
  _end = .;
}
```

首先我们使用OUTPUT_ARCH指定了架构为RISC-V，之后使用ENTRY指定程序入口点为`_start`函数，程序入口点即程序启动时运行的函数，经过这样的指定后在head.S中需要编写`_start`函数，程序才能正常运行。

链接脚本中有`.` `*`两个重要的符号。单独的`.`在链接脚本代表当前地址，它有赋值、被赋值、自增等操作。而`*`有两种用法，其一是`*()`在大括号中表示将所有文件中符合括号内要求的段放置在当前位置，其二是作为通配符。

链接脚本的主体是SECTIONS部分，在这里链接脚本的工作是将程序的各个段按顺序放在各个地址上，在例子中就是从0x80000000地址开始放置了.text，.rodata，.data和.bss段。各个段的作用可以简要概括成：

| 段名    | 主要作用                             |
| ------- | ------------------------------------ |
| .text   | 通常存放程序执行代码                 |
| .rodata | 通常存放常量等只读数据               |
| .data   | 通常存放已初始化的全局变量、静态变量 |
| .bss    | 通常存放未初始化的全局变量、静态变量 |

在链接脚本中可以自定义符号，例如stack_top与_end都是我们自己定义的，其中stack_top与程序调用栈有关。

更多有关链接脚本语法可以参考[这里](https://sourceware.org/binutils/docs/ld/Scripts.html)。

#### Image

在Linux Kernel开发中，为了对vmlinux进行精简，通常使用objcopy丢弃调试信息与符号表并生成二进制文件，这就是Image。Lab0中QEMU正是使用了Image而不是vmlinux运行。

```shell
$ objcopy -O binary vmlinux Image --strip-all
```

此时再对Image使用**file**命令时：

```shell
$ file Image 
image: data
```

