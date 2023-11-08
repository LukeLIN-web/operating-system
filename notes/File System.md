## File System Interface

- proc文件系统
  - 没有真正的文件做backup，就是一个接口
  - 用户态程序能够获得系统的状态信息
  - 内核会把当时实时状态信息返回
- metadata是事先连续存放的
  - 如果来一个文件才创一份metadata，那么这些metadata不连续，速度慢
- 创建链接
  - by default：软链接（符号链接）

#### Open Files

- 维护两个表
  - Per-process table: current location pointer, access rights
    - 这个进程open了哪些文件，fd就是表里的index
  - System-wide table: location on the disk
  - per-process表里需要一个指针指向system-wide表，指明这个文件在物理内存的位置









