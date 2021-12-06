#include "put.h"
#include "sched.h"
#define offset (0xffffffe000000000 - 0x80000000)

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 4, 3, 2, 1};

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);
extern unsigned int rand();
extern uint64_t cur;

void task_init(void)
{
    puts("task init...\n");

    //initialize task[0]
    current = (struct task_struct *)0xffffffe000210000;
    current->state = TASK_RUNNING;
    current->counter = 1;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;
    task[0] = current;
    task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;
    //set other 4 tasks
    for (int i = 1; i <= LAB_TEST_NUM; ++i)
    {
        struct task_struct *tmp = (struct task_struct *)(0xffffffe000210000 + PAGE_SIZE * i);
        tmp->state = TASK_RUNNING;
        tmp->counter = COUNTER_INIT_COUNTER[i];
        tmp->priority = PRIORITY_INIT_COUNTER[i];
        tmp->blocked = 0;
        tmp->pid = i;
        task[i] = tmp;
        task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
        task[i]->thread.ra = &init_epc;
        /*your code*/
        puts("[PID = ");
        puti(task[i]->pid);
        puts("] Process Create Successfully! counter = ");
        puti(task[i]->counter);
        puts(" priority = ");
        puti(task[i]->priority);
        puts("\n");
    }
}

void do_timer(void)
{
    //current process's counter -1, judge whether to schedule or go on.
    /*your code*/
    
    puts("[*PID = ");
    puti(current->pid);
    puts("] Context Calculation: counter = ");
    puti(current->counter);
    puts("\n");
    current->counter--;
    if (current->counter == 0)
        schedule();
}

void schedule(void)
{
    int next = -1;
    long min = 999;
    for (int i = LAB_TEST_NUM; i >= 1; i--)
    {
        if (task[i]->counter == 0)
            continue;
        if (task[i]->counter < min)
        {
            next = i;
            min = task[next]->counter;
        }
    }
    if (next == -1)
    {
        puts("Finished All Task, Reallocate Counter For ALL TASK\n");
        int tmp = LAB_TEST_NUM;
        int min = 999;
        while (tmp >= 1){   
            task[tmp]->counter = (rand() % 5);
            if (task[tmp]->counter > 0 && task[tmp]->counter < min)
            {
                next = tmp;
                min = task[next]->counter;
            }
            puts("[*PID = ");
            puti(tmp);
            puts("] Reset counter = ");
            puti(task[tmp]->counter);
            puts("\n");
            tmp--;
        }
        switch_to(task[next]);
        return ;
    }
    else{
        if (next != -1 && current->pid != task[next]->pid)
        {
            puts("[!] Switch from task ");
            puti(current->pid);
            puts(" [task struct: ");
            putullHex((unsigned long long)current);
            puts(", sp: ");
            putullHex((unsigned long long)current->thread.sp);
            puts("] to task ");
            puti(task[next]->pid);
            puts(" [task struct: ");
            putullHex((unsigned long long)next);
            puts(", sp: ");
            putullHex((unsigned long long)task[next]->thread.sp);
            puts("], prio: ");
            puti(task[next]->priority);
            puts(", counter: ");
            puti(task[next]->counter);
            puts("\n");
        }
        switch_to(task[next]);
    }
}

void switch_to(struct task_struct *next)
{
    if (next->pid != current->pid)
    {
        struct task_struct *prev = current;
        current = next;
        __switch_to(prev, current);
    }
}

void dead_loop(void)
{
    while (1)
    {
        continue;
    }
}