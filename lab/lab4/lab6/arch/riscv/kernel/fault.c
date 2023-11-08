#include "fault.h"

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
    //注意需要特判fork⼦进程的栈⻚和进程的代码段，这时不需要⽤kmalloc来获取物理⻚，前者通过映射到已复制的⼦进程物理⻚，后者映射到0x84000000
    unsigned long pa;
    unsigned long sz = (found->vm_end - found->vm_start + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
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