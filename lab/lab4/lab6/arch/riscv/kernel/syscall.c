#include "syscall.h"
#include "buddy.h"
unsigned long sys_write(unsigned int fd, const char* buf, size_t count) {
    if (fd == 1) {
        puts(buf);
        return count;
    }
}

unsigned long sys_getpid() {
    return current->pid;
}

#define __off_t unsigned long

//我们采用最简单的暴力搜索方法来寻找未映射的长度为 `length` (按页对齐) 的虚拟地址区域：
//从0地址开始向上以 `PAGE_SIZE`为单位遍历，直到遍历到连续 `length`长度内均无已有映射的地址区域，将该区域的首地址返回。
unsigned long get_unmapped_area(size_t length) {
    unsigned long i = 0;
    for (i = 0; ; ++i) {
        struct vm_area_struct *ptr;
        char flag = 0;
        for (ptr = current->thread.mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
            if (ptr->vm_start < i + length && ptr->vm_end > i) {
                flag = 1;
                break;
            }
        }
        if (flag == 0) {
            return i;
        }
    }

}

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

extern void ret_from_fork(unsigned long *stack);

void forkret() {
    asm volatile ("csrrw x0, sscratch, %0" : : "r"(current->thread.sscratch));
    ret_from_fork(current->thread.stack);
}


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

    task[task_num_top]->thread.sepc = current->thread.sepc;
    task[task_num_top]->thread.ra = (unsigned long)forkret;
    task[task_num_top]->thread.sscratch = sscratch_top;
    task[task_num_top]->thread.sp = sscratch_top - 280;
    task[task_num_top]->thread.mm = kmalloc(sizeof(struct mm_struct));
    task[task_num_top]->thread.mm->pgtbl = (uint64)kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
    task[task_num_top]->thread.mm->vm_area_head = NULL;
    task[task_num_top]->thread.mm->pa_for_stack = 0;
    uint64 pgtbl_va = (uint64)task[task_num_top]->thread.mm->pgtbl;
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
    task[task_num_top]->thread.mm->pa_for_stack = (uint64)new_stack - page_offset + 0x80000000ul;
    for (int i = 0; i < PAGE_SIZE; ++i) {
        new_stack[i] = ((char*)(USER_END - PAGE_SIZE))[i];
    }
    
    return task_num_top;// `fork`成功时 `返回：子进程的pid`
}