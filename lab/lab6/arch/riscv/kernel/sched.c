#include "sched.h"
#include "rand.h"
#include "put.h"
#include "vm.h"
#include "stddef.h"
#include "slub.h"
#include "syscall.h"
#include "buddy.h"

struct task_struct *current;
struct task_struct * task[NR_TASKS];
unsigned long long BASE_ADDR = 0xffffffe000000000;


#define MAX_INT 0x7fffffff

#define INIT_SPTOP 0xffffffdf80000000
unsigned long USER_END = 0xffffffdf80000000ul;


unsigned long task_num_top = LAB_TEST_NUM;
extern void switch_context(struct thread_struct *, struct thread_struct *);
extern void task_first_ret();

void initial_pgtbl(unsigned long pgtbl) {
    unsigned long text_start = kmem_struct.text_start, rodata_start = kmem_struct.rodata_start, data_start = kmem_struct.data_start;
    unsigned long virtual_offset = kmem_struct.virtual_offset;
    create_mapping(pgtbl, virtual_offset + text_start, text_start, rodata_start - text_start, 5 << 1);
    create_mapping(pgtbl, virtual_offset + rodata_start, rodata_start, data_start - rodata_start, 1 << 1);
    create_mapping(pgtbl, virtual_offset + data_start, data_start, 0x1000000 - (data_start - text_start), 3 << 1);

    create_mapping(pgtbl, 0xffffffdf90000000, 0x10000000, 0x1000, 3 << 1);
}

void initial_kernel_stack(unsigned long sscratch_top, unsigned long sp) {
    __asm__ __volatile__("addi sp, sp, -16\n\t");
    __asm__ __volatile__("sd s1, 0(sp)\n\t");
    __asm__ __volatile__("sd s2, 8(sp)\n\t");

    // __asm__ __volatile__("csrrw s1, sscratch, s1\n\t");
    __asm__ __volatile__("addi s1, %0, -280\n\t": : "r"(sscratch_top));

    __asm__ __volatile__("sd %0, 8(s1)\n\t" : : "r"(sp));

    __asm__ __volatile__("ld s1, 0(sp)\n\t");
    __asm__ __volatile__("ld s2, 8(sp)\n\t");
    __asm__ __volatile__("addi sp, sp, 16\n\t");
}

void task_init(void) {
    puts("task init...\n");
    task[0] = kmalloc(sizeof(struct task_struct));
    unsigned long long sscratch_top;
    current = task[0];
    task[0]->state = TASK_RUNNING;
    task[0]->counter = 0;
    task[0]->priority = 5;
    task[0]->blocked = 0;
    task[0]->pid = 0;
    task[0]->thread.sp = INIT_SPTOP;
    sscratch_top = kmalloc(PAGE_SIZE) + PAGE_SIZE;
    task[0]->thread.sscratch = sscratch_top;
    task[0]->thread.sepc = 0;
    task[0]->thread.mm = kmalloc(sizeof(struct mm_struct));
    task[0]->thread.mm->pgtbl = (unsigned long)kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
    task[0]->thread.mm->vm_area_head = NULL;

    task[1] = kmalloc(sizeof(struct task_struct));
    
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

    do_mmap(task[1]->thread.mm, 0, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    do_mmap(task[1]->thread.mm, USER_END - PAGE_SIZE, PAGE_SIZE, PROT_READ | PROT_WRITE);

    #ifdef SJF
    puts("[ PID = ");
    puti(task[1]->pid);
    puts("] Process Create Successfully! counter = ");
    puti(task[1]->counter);
    puts("\n");
    #endif
    #ifdef PRIORITY
    puts("[ PID = ");
    puti(task[1]->pid);
    puts("] Process Create Successfully! counter = ");
    puti(task[1]->counter);
    puts(" priority = ");
    puti(task[1]->priority);
    puts("\n");
    #endif
    // __asm__ __volatile__("csrrw x0, sscratch, %0" :: "r"(task[0]->thread.sscratch));

}

void do_timer(void) {
    
    #ifdef SJF
    puts("[ PID = ");
    puti(current->pid);
    puts("] Context Calculation: counter = ");
    puti(current->counter);
    puts("\n");
    if (current->counter > 0) {
        --current->counter;
    }
    if (current->counter == 0) {
        schedule();
    }
    #endif
    #ifdef PRIORITY
    
    if (current->counter > 0) {
        --current->counter;
    }
    if (current != task[0] && current->counter == 0) {
        current->counter = 8 - current->pid;
    }
    schedule();
    #endif

}

void puti64(unsigned long x) {
    int a[16];
    int i = 0;
    while (x) {
        a[i] = x % 16;
        ++i;
        x /= 16;
    }
    for (i = 15; i >= 0; --i) {
        switch (a[i]) {
          case 0: puts("0"); break;
          case 1: puts("1"); break;
          case 2: puts("2"); break;
          case 3: puts("3"); break;
          case 4: puts("4"); break;
          case 5: puts("5"); break;
          case 6: puts("6"); break;
          case 7: puts("7"); break;
          case 8: puts("8"); break;
          case 9: puts("9"); break;
          case 10: puts("A"); break;
          case 11: puts("B"); break;
          case 12: puts("C"); break;
          case 13: puts("D"); break;
          case 14: puts("E"); break;
          case 15: puts("F"); break;
        }
    }
}

void schedule(void) {

    #ifdef SJF
    int mintime = MAX_INT;
    int mint = -1;
    for (int i = task_num_top; i > 0; --i) {
        if (task[i]->counter > 0 && task[i]->counter < mintime) {
            mintime = task[i]->counter;
            mint = i;
        }
    }

    if (mint == -1) {
        // puts("task init...\n");
        for (int i = 1; i <= task_num_top; ++i) {
            task[i]->counter = rand();
            puts("[ PID = ");
            puti(task[i]->pid);
            puts("] Reset counter = ");
            puti(task[i]->counter);
            puts("\n");
        }
        // current = task[0];
        schedule();

        // task_init();
        // schedule();
        return;
    }
    switch_to(task[mint]);

    #endif
    #ifdef PRIORITY
    int mintime = MAX_INT;
    int maxprio = MAX_INT;
    int mint = -1;
    for (int i = task_num_top; i > 0; --i) {
        if (task[i]->counter > 0 && task[i]->priority < maxprio) {
            maxprio = task[i]->priority;
            mintime = task[i]->counter;
            mint = i;
        } else if (task[i]->counter > 0 && task[i]->priority == maxprio && task[i]->counter < mintime) {
            mintime = task[i]->counter;
            mint = i;
        }
    }
    if (task[mint] != current) {
        puts("[!] Switch from task ");
        puti(current->pid);
        puts(" [task struct: 0x");
        puti64(current);
        puts(", sp: 0x");
        puti64(current->thread.sp);
        puts("] to task ");
        puti(task[mint]->pid);
        puts(" [task struct: 0x");
        puti64(task[mint]);
        puts(", sp: 0x");
        puti64(task[mint]->thread.sp);
        puts("], prio: ");
        puti(task[mint]->priority);
        puts(", counter: ");
        puti(task[mint]->counter);
        puts("\n");
    }

    puts("tasks' priority changed\n");
    for (int i = 1; i <= task_num_top; ++i) {
        task[i]->priority = rand();
        puts("[ PID = ");
        puti(task[i]->pid);
        puts("] counter = ");
        puti(task[i]->counter);
        puts(" priority = ");
        puti(task[i]->priority);
        puts("\n");
    }
    switch_to(task[mint]);
    #endif
}

void switch_to_asm(struct task_struct* next) {

    __asm__ __volatile__("add %0, ra, x0\n\t"
		:"=r"(current->thread.ra)
		);
    __asm__ __volatile__("add %0, sp, x0\n\t"
		:"=r"(current->thread.sp)
		);
    __asm__ __volatile__("add %0, s0, x0\n\t"
		:"=r"(current->thread.s0)
		);
    __asm__ __volatile__("add %0, s1, x0\n\t"
		:"=r"(current->thread.s1)
		);
    __asm__ __volatile__("add %0, s2, x0\n\t"
		:"=r"(current->thread.s2)
		);
    __asm__ __volatile__("add %0, s3, x0\n\t"
		:"=r"(current->thread.s3)
		);
    __asm__ __volatile__("add %0, s4, x0\n\t"
		:"=r"(current->thread.s4)
		);
    __asm__ __volatile__("add %0, s5, x0\n\t"
		:"=r"(current->thread.s5)
		);
    __asm__ __volatile__("add %0, s6, x0\n\t"
		:"=r"(current->thread.s6)
		);
    __asm__ __volatile__("add %0, s7, x0\n\t"
		:"=r"(current->thread.s7)
		);
    __asm__ __volatile__("add %0, s8, x0\n\t"
		:"=r"(current->thread.s8)
		);
    __asm__ __volatile__("add %0, s9, x0\n\t"
		:"=r"(current->thread.s9)
		);
    __asm__ __volatile__("add %0, s10, x0\n\t"
		:"=r"(current->thread.s10)
		);
    __asm__ __volatile__("add %0, s11, x0\n\t"
		:"=r"(current->thread.s11)
		);
    __asm__ __volatile__("csrr t0, sepc\n\tadd %0, t0, x0\n\t"
		:"=r"(current->thread.sepc)
		);
    __asm__ __volatile__("csrr t0, sscratch\n\tadd %0, t0, x0\n\t"
		:"=r"(current->thread.sscratch)
		);
    // __asm__ __volatile__("addi t0, t0, 1\n\tslli t0, t0, 44\n\taddi t0, t0, -1\n\tcsrr t1, satp\n\tand t1, t1, t0\n\tsll %0, t1, 12\n\t"
	// 	:"=r"(current->thread.mm->pgtbl)
	// 	);
    
    __asm__ __volatile__("add ra, %0 ,x0\n\t"
		: :"r"(next->thread.ra)
		);
    __asm__ __volatile__("add sp, %0 ,x0\n\t"
		: :"r"(next->thread.sp)
		);
    __asm__ __volatile__("add s0, %0 ,x0\n\t"
		: :"r"(next->thread.s0)
		);
    __asm__ __volatile__("add s1, %0 ,x0\n\t"
		: :"r"(next->thread.s1)
		);
    __asm__ __volatile__("add s2, %0 ,x0\n\t"
		: :"r"(next->thread.s2)
		);
    __asm__ __volatile__("add s3, %0 ,x0\n\t"
		: :"r"(next->thread.s3)
		);
    __asm__ __volatile__("add s4, %0 ,x0\n\t"
		: :"r"(next->thread.s4)
		);
    __asm__ __volatile__("add s5, %0 ,x0\n\t"
		: :"r"(next->thread.s5)
		);
    __asm__ __volatile__("add s6, %0 ,x0\n\t"
		: :"r"(next->thread.s6)
		);
    __asm__ __volatile__("add s7, %0 ,x0\n\t"
		: :"r"(next->thread.s7)
		);
    __asm__ __volatile__("add s8, %0 ,x0\n\t"
		: :"r"(next->thread.s8)
		);
    __asm__ __volatile__("add s9, %0 ,x0\n\t"
		: :"r"(next->thread.s9)
		);
    __asm__ __volatile__("add s10, %0 ,x0\n\t"
		: :"r"(next->thread.s10)
		);
    __asm__ __volatile__("add s11, %0 ,x0\n\t"
		: :"r"(next->thread.s11)
		);
    __asm__ __volatile__("add t0, %0, x0\n\tcsrrw x0, sepc, t0\n\t"
		: :"r"(next->thread.sepc)
		);
    __asm__ __volatile__("add t0, %0, x0\n\tcsrrw x0, sscratch, t0\n\t"
		: :"r"(next->thread.sscratch)
		);
    __asm__ __volatile__("addi t0, t0, 1\n\tslli t0, t0, 63\n\tsrl t1, %0, 12\n\tor t0, t0, t1\n\tcsrrw x0, satp, t0\n\t"
		: :"r"(next->thread.mm->pgtbl)
		);
    
    current = next;
}

void switch_to(struct task_struct* next) {
    if (next == current) {
        return;
    }
    #ifdef SJF
    puts("[!] Switch from task ");
    puti(current->pid);
    puts(" to task ");
    puti(next->pid);
    puts(", prio: ");
    puti(next->priority);
    puts(", counter: ");
    puti(next->counter);
    puts("\n");
    #endif

    // puti(has_page_tables(next->thread.mm->pgtbl, 0x0));
    // puts("\n");

    struct thread_struct *content_old = &current->thread;
    struct thread_struct *content_new = &next->thread;

    __asm__ __volatile__("addi t0, x0, 1\n\tslli t0, t0, 63\n\tsrl t1, %0, 12\n\tor t0, t0, t1\n\tcsrrw x0, satp, t0\n\t"
		: :"r"(next->thread.mm->pgtbl)
		);
    __asm__ __volatile__ ("sfence.vma");
    current = next;

    switch_context(content_old, content_new);
}

void dead_loop(void) {
    while(1) ;
}