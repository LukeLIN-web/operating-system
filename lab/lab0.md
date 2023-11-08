# Lab 0: RV64 内核调试

## 1 实验目的

安装虚拟机及Docker，通过在QEMU模拟器上运行Linux来熟悉如何从源代码开始将内核运行在QEMU模拟器上，学习使用GDB跟QEMU对代码进行联合调试，为后续实验打下基础。

## 2 实验内容及要求

* 安装虚拟机软件、Ubuntu镜像，自行学习Linux基础命令。
* 安装Docker，下载并导入Docker镜像，创建并运行容器。

* 编译内核并用 gdb + QEMU 调试，在内核初始化过程中设置断点，对内核的启动过程进行跟踪，并尝试使用gdb的各项命令。

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，**跟随实验步骤完成实验并以截图的方式记录实验过程**，如有需要请对每一步的命令以及结果进行必要的解释。

## 3 操作方法和实验步骤

### 3.1 通过虚拟机安装Linux系统(20%)

请参考【附录A.LINUX 基础】了解相关背景知识。

##### **1.下载并安装虚拟机软件**

 VMware Workstation Player：[VMware Workstation Player - VMware Customer Connect](https://customerconnect.vmware.com/cn/downloads/info/slug/desktop_end_user_computing/vmware_workstation_player/16_0#product_downloads)

##### **2.下载Ubuntu镜像**

建议下载Ubuntu18.04/ubuntu-18.04.5-desktop-amd64.iso：[Index of /ubuntu-releases/18.04/ (zju.edu.cn)](https://mirrors.zju.edu.cn/ubuntu-releases/18.04/)

##### **3.通过虚拟机软件安装Ubuntu镜像**

自行搜索相应教程进行安装，建议将用户名设置为姓名缩写，**注意将硬盘大小设置到30G以上方便后续实验**。

##### **4.自行学习Linux命令行的使用，了解基本命令**

推荐资源：

* [课程概览与 shell · the missing semester of your cs education (missing-semester-cn.github.io)](https://missing-semester-cn.github.io/2020/course-shell/)（这门课程的其他小节也建议大家学习一下）
* [TLCL (billie66.github.io)](http://billie66.github.io/TLCL/book/index.html)（建议阅读第二~五章、第十二章、十三章相关内容）

请依照第一行的例子，解释以下命令并附上运行截图（推荐截图工具：[Snipaste](https://www.snipaste.com)），适当缩放图片大小。

| 命令                    | 作用                                                      | 截图                                                         |
| ----------------------- | --------------------------------------------------------- | ------------------------------------------------------------ |
| pwd                     | 打印出当前工作目录的名称                                  | ![image-20210913102609974](https://raw.githubusercontent.com/HHtttterica/Pics/main/image-20210913102609974.png) |
| ls                      | 打印当前工作目录下所有文件                                | ![image-20210928231013183](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231013183.png) |
| ls -al                  | 列出目录下的所有文件详细资料，包括以 `.` 开头的隐含文件。 | ![image-20210928231024490](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231024490.png) |
| cd ~                    | 切换当前目录至~                                           | ![image-20210928231032858](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231032858.png) |
| mkdir oslab             | 创建 oslab 文件夹                                         | ![image-20210928231327452](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231327452.png) |
| vi test.c               | 编辑test.c 文件                                           | ![image-20210928231505404](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231505404.png) |
| gedit test.c            | 编辑test.c文件                                            | wsl无法显示GUI.                                              |
| rm test.c               | 删除test.c                                                | ![image-20210928231523520](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231523520.png) |
| sudo apt  install curl  | 安装curl                                                  | ![image-20210928231538139](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231538139.png) |
| touch a.txt             | 创建 a.txt                                                | 如下                                                         |
| cat a.txt \| tail -n 10 | 打印 a.txt的最后10行                                      | ![image-20210928231724298](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210928231724298.png) |

### 3.2 安装 Docker 环境并创建容器(20%)

请参考【附录B.Docker使用基础】了解相关背景知识。

通常情况下，`$` 提示符表示当前运行的用户为普通用户，`#` 代表当前运行的用户为特权用户。但在下文的示例中，以 `###` 开头的行代表注释，`$` 开头的行代表在虚拟机上运行的命令，`#` 开头的行代表在 `docker` 中运行的命令，`(gdb)` 开头的行代表在 `gdb` 中运行的命令。

**在执行每一条命令前，请你对将要进行的操作进行思考，给出的命令不需要全部执行，并且不是所有的命令都可以无条件执行，请不要直接复制粘贴命令去执行。**

##### **1.安装docker**

```shell
### 使用官方安装脚本自动安装docker
$ curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun

### 将用户加入docker组，免 sudo
$ sudo usermod -aG docker $USER   ### 注销后重新登陆生效
```

**【请将该步骤中命令的输入及运行结果截图附在此处，示例如下。接下来每一步同理，不作赘述，】**

因为之前在windows 安装了docker, 所以不再重复安装.

##### 2.下载并导入docker镜像

为便于开展实验，我们在[docker镜像](https://pan.zju.edu.cn/share/d10c6d076e60a89964df83a178)中提前安装好了实验所需的环境（RISC-V工具链、QEMU模拟器），并设置好了`$PATH`变量以直接访问`qemu-system-riscv64`等程序。请下载该docker镜像文件并将其传至虚拟机中（可以直接拖拽到文件夹中），输入命令导入镜像。

```bash
### 首先进入oslab.tar所在的文件夹，然后使用该命令导入docker镜像
$ cat oslab.tar | docker import - oslab:2020
### 执行命令后若出现以下错误提示
### ERROR: Got permission denied while trying to connect to the Docker daemon socket at unix:///var/run/docker.sock
### 可以使用下面命令为该文件添加权限来解决
### $ sudo chmod a+rw /var/run/docker.sock

### 查看docker镜像
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
oslab               2020                d7046ea68221        5 seconds ago       2.89G
```

我下载了oslab.tar , 然后导入到之前安装的windows docker中

 ```shell
 docker import C:\Users\12638\Downloads\oslab.tar oslab:2020
 ```

![image-20210919225255214](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919225255214.png)

上图可以看出导入成功 ,但是没有tag. 用`docker tag`命名一下.

##### 3.从镜像中创建一个容器并进入该容器

```shell
### 从镜像创建一个容器
$ docker run -it oslab:2020 /bin/bash    ### -i:交互式操作 -t:终端
root@368c4cc44221:/#                     ### 提示符变为 '#' 表明成功进入容器 后面的字符串根据容器而生成，为容器id
root@368c4cc44221:/# exit (或者CTRL+D）   ### 从容器中退出 此时运行docker ps，运行容器的列表为空

### 查看当前运行的容器
$ docker ps 
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
### 查看所有存在的容器
$ docker ps -a 
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS                      PORTS               NAMES
368c4cc44221        oslab:2020          "/bin/bash"         54 seconds ago      Exited (0) 30 seconds ago                       relaxed_agnesi

### 启动处于停止状态的容器
$ docker start 368c     ### 368c 为容器id的前四位，id开头的几位便可标识一个容器
$ docker ps             ### 可看到容器已经启动
CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
368c4cc44221        oslab:2020          "/bin/bash"         About a minute ago   Up 16 seconds                           relaxed_agnesi

### 进入已经运行的容器 oslab的密码为2020
$ docker exec -it -u oslab -w /home/oslab 36 /bin/bash
oslab@368c4cc44221:~$
```

我创建容器,按上述操作了一遍.

```shell
docker run -it ljy/oslab:v1 /bin/bash
docker start cf0e   
docker exec -it -u oslab -w /home/oslab cf /bin/bash
```

![image-20210919230039658](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919230039658.png)

可以看到都成功了.

**请解释这两行命令：**

* `docker run -it oslab:2020 /bin/bash`

  答：`docker run`指创建一个新的容器并运行一个命令，`-it`指交互模式，`oslab:2020`指定了所使用的镜像，`/bin/bash`指在容器内执行`/bin/bash`命令。

* `docker exec -it -u oslab -w /home/oslab 36 /bin/bash`

  答： `docker exec`指Run a command in a running container. `-it`指交互模式，-u 指定了username, -w 指定了working directory. 36是container 的id, 我这里是cf, /bin/bash是运行bash.


### 3.3 编译 linux 内核(20%)

请参考【附录E.LINUX 内核编译基础】了解相关背景知识。

再次强调，以 `###` 开头的行代表注释，`$` 开头的行代表在虚拟机上运行的命令，**`#` 开头的行代表在 `docker` 中运行的命令，请勿将`#`一并复制入命令行中执行。**

```shell
### 进入实验目录并设置环境变量
# pwd
/home/oslab
# cd lab0
# export TOP=`pwd`
# mkdir -p build/linux
# make -C linux O=$TOP/build/linux \
          CROSS_COMPILE=riscv64-unknown-linux-gnu- \
          ARCH=riscv CONFIG_DEBUG_INFO=y \
          defconfig all -j$(nproc)
### 编译内核需要一定时间，请耐心等待。
```

编译了大约十分钟

![image-20210919231759767](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919231759767.png)

### 3.4 使用QEMU运行内核(20%)

请参考【附录C.QEMU使用基础】了解相关背景知识。

**注意，QEMU的退出方式较为特殊，需要先按住`ctrl+a`，放开后再按一次`x`。**

```shell
### 用户名root，没有密码
# qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0
```

![image-20210919232251994](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919232251994.png)

![image-20210919232432225](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919232432225.png)

登录成功

### 3.5 使用 gdb 调试内核(20%)

请参考【附录D.GDB使用基础】了解相关背景知识。学会调试将在后续实验中为你提供帮助，推荐同学们跟随[GDB调试入门指南](https://zhuanlan.zhihu.com/p/74897601)教程完成相应基础练习，熟悉gdb调试的使用。

```shell
### Terminal 1
# qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0 -S -s
```

再打开一个终端窗口，从宿主机进入该容器，并切换到`lab0`文件夹，进入gdb调试。

```shell
###Terminal2
# riscv64-unknown-linux-gnu-gdb build/linux/vmlinux
```

如下图, 在第二个terminal 进入gdb调试

![image-20210919233213448](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919233213448.png)

 顺序执行下列gdb命令，写出每条命令的含义并附上执行结果的截图。

```shell
(gdb) target remote localhost:1234
```

* 含义：target remote 命令表示远程调试，而1234是默认的用于调试连接的端口号。

* 执行结果：

  ![image-20210919233508715](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919233508715.png)

```shell
(gdb) b start_kernel 
(gdb) b *0x80000000
(gdb) b *0x80200000
(gdb) info breakpoints
(gdb) delete 2
(gdb) info breakpoints
```

* 含义： 设置三个断点 , 查看已设置断点信息, 删除第二个断点,再次查看已设置断点信息
* 执行结果：

![image-20210919233805942](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919233805942.png)

``` shell
(gdb) continue #继续执行程序，直到再次遇到断点处：
(gdb) delete 3 
(gdb) continue
(gdb) step #如果我们想跟踪函数内部的情况，可以使用step命令（可简写为s），它可以单步跟踪到函数内部，但前提是该函数有调试信息并且有源码信息。
(gdb) s #step的简写
(gdb) (不做输入，直接回车) # 重复上一次命令s
(gdb) next #用于在程序断住后，继续执行下一条语句，假设已经启动调试，并在第12行停住，如果要继续执行，则使用n执行下一条语句
(gdb) n #next的简写.
(gdb) (不做输入，直接回车)# 重复上一次命令n
```

* 含义：写在注释中

* 执行结果：

  ![image-20210919234837643](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919234837643.png)

```shell
(gdb) disassemble # 查看汇编代码,不带参数：默认的反汇编范围是所选择地址附近的汇编代码；
(gdb) nexti #每次执行一条机器指令,不进入函数
(gdb) n # next的简写. 
(gdb) stepi #每次执行一条机器指令,进入函数.
(gdb) s # step的简写
```

* 含义：如上

* 执行结果：

  ![image-20210919235116018](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919235116018.png)

  

  ![image-20210919235503688](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919235503688.png)

  

* **请回答：nexti和next的区别在哪里？stepi和step的区别在哪里？next和step的区别是什么？**

  答： next是源代码的下一行,不进入函数, nexti是 单步机器指令,不进入函数
  
   step是单步到下一个不同的源代码行, 包括进入函数, stepi是单步机器指令.  next 不会进入到函数内部, step会进入到函数内部 

```shell
(gdb) continue #继续执行程序，直到再次遇到断点处：
(gdb) quit  #退出gdb
```

* 含义： 如上
* 执行结果：

![image-20210919235720946](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919235720946.png)

没有断点了, 机器就启动了 . 

![image-20210920000345465](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210920000345465.png)

## 4 讨论和心得

请在此处填写实验过程中遇到的问题及相应的解决方式。由于本实验为新实验，可能存在不足之处，欢迎同学们对本实验提出建议。

- 3.2.2 出现问题 “<”运算符是为将来使用而保留的。 

`docker load < C:\Users\12638\Downloads\oslab.tar` 谷歌一下, 因为powershell不支持, 尝试用cmd可以解决.

- 3.2.2出现问题open /var/lib/docker/tmp/docker-import-673032073/bin/json: no such file or directory**因为压缩包如果是用 docker save 打包的，就可以用 docker load，但是如果压缩包是用 docker export 打包的，那就需要用 docker import** 

`cat oslab.tar | docker import - ljy/oslab:2021 # 这是不行的cmd没有cat`

过程中查阅了https://docs.docker.com/engine/reference/commandline/import/

- 在disassemble这里直接输入nexti似乎没用.

![image-20210919235228879](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210919235228879.png)

我就先q退出了, 再输入nexti.

- continue后, 程序就一直运行.

可以看到terminal1 一开始是阻塞的,`gdb continue`后可以登录了. 为了让程序停下来，我们可以发送一个中断信号（ctrl + c），GDB捕捉到该信号后会挂起被调试进程。

![image-20210920000121428](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210920000121428.png)

这个时候 terminal1也不能输入了.

再次continue后, 会把之前terminal1 的输入给显示出来. 

![image-20210920000312499](C:\Users\12638\AppData\Roaming\Typora\typora-user-images\image-20210920000312499.png)

