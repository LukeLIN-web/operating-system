#include "fault.h"

void do_page_fault() {
    
    unsigned long scause;
    __asm__ __volatile__ ("csrr %0, scause\n\t" : "=r" (scause));

    unsigned long sepc;
    __asm__ __volatile__ ("csrr %0, sepc\n\t" : "=r" (sepc));

    unsigned long stval;
    __asm__ __volatile__ ("csrr %0, stval\n\t" : "=r" (stval));
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
    unsigned pa;
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