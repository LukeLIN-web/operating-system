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

unsigned long get_unmapped_area(size_t length) {
    unsigned long i = 0;
    for (i = 0; ; ++i) {
        struct vm_area_struct *ptr;
        int flag = 0;
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
    for (ptr = mm->vm_area_head; ptr != NULL; ptr = ptr->vm_next) {
        if (ptr->vm_start < (unsigned long)start + length && ptr->vm_end > (unsigned long)start) {
            start = get_unmapped_area(length);
            break;
        }
    }
    struct vm_area_struct *new_vm_area = kmalloc(sizeof(struct vm_area_struct));
    new_vm_area->vm_start = (unsigned long)start;
    new_vm_area->vm_end = start + length;
    new_vm_area->vm_flags = prot;
    new_vm_area->vm_mm = mm;
    new_vm_area->vm_page_prot.pgprot = ((prot & PROT_READ ? 1 : 0) | (prot & PROT_WRITE ? 2 : 0) | (prot & PROT_EXEC ? 4 : 0)) << 1;
    if (mm->vm_area_head == NULL) {
        mm->vm_area_head = new_vm_area;
    } else {
        for (ptr = mm->vm_area_head; ptr->vm_next != NULL; ptr = ptr->vm_next) ;
        ptr->vm_next = new_vm_area;
        new_vm_area->vm_prev = ptr;
    }
    return start;
}

void *mmap (void *__addr, size_t __len, int __prot,
            int __flags, int __fd, __off_t __offset) {
    return do_mmap(current->thread.mm, __addr, __len, __prot);
}

int munmap(void *start, size_t length) {
    struct vm_area_struct *found = NULL;
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
    free_page_tables(current->thread.mm->pgtbl, start, length, 1);
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
    #endif
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

    for (int i = 0; i < 280 / 8; ++i) {
        task[task_num_top]->thread.stack[i] = current->thread.stack[i];
    }
    task[task_num_top]->thread.stack[9] = 0;
    char *new_stack = kmalloc(PAGE_SIZE);
    task[task_num_top]->thread.mm->pa_for_stack = (unsigned long)new_stack - page_offset + 0x80000000ul;
    for (int i = 0; i < PAGE_SIZE; ++i) {
        new_stack[i] = ((char*)(USER_END - PAGE_SIZE))[i];
    }
    
    return task_num_top;
}