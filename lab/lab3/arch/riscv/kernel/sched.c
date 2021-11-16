#include "stdio.h"
#include "sched.h"

#define Kernel_Page            0x80210000
#define LOW_MEMORY             0x80211000
#define PAGE_SIZE              4096UL
 

struct task_struct* current;
struct task_struct* task[NR_TASKS];


extern task_test_done;
extern void __init_sepc(void);

//If next==current,do nothing; else update current and call __switch_to.
void switch_to(struct task_struct* next) {
    /*your code*/
    if(next->pid != current->pid){
        current->pid = next->pid;
        __switch_to(current,next);
    }
}

int task_init_done = 0;
//initialize tasks, set member variables
void task_init(void) {
    puts("task init...\n");

    //initialize task[0]
    current = (struct task_struct*)Kernel_Page;
    current->state = TASK_RUNNING;
    current->counter = 1;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;
    task[0] = current;
    task[0]->thread.sp = (unsigned long long) task[0] + TASK_SIZE; //设置`current`指向`Task0 Space`的基地址。

    //set other 4 tasks
    for (int i = 1; i <= LAB_TEST_NUM; ++i) {
        /*your code*/
        current = (struct task_struct*)(Kernel_Page+PAGE_SIZE*i);
        current->state = TASK_RUNNING;
        current->counter = 1;
        current->priority = 5;
        current->blocked = 0;
        current->pid = i;
        task[i] = current;
        task[i]->thread.sp = (unsigned long long) task[i] + TASK_SIZE; //设置`current`指向`Task0 Space`的基地址。
        task[i]->thread.ra = __init_sepc;
        printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
    }
    task_init_done = 1;
}


#ifdef SJF 
//simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void) {
    if (!task_init_done) return;
    if (task_test_done) return;
    printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter,current->priority);
    //current process's counter -1, judge whether to schedule or go on.
    /*your code*/
    current->counter --;
    if(current->counter == 0){
        schedule();
    }
    else{
        return;
    }

}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    unsigned char next = 63 ; 
    /*your code*/
    int i;
    int c = task[63]->counter;
    for(i = 62;i >=1;i --){
        c += task[i]->counter;
        if(task[i]->counter < task[next]->counter){
            next = i;
        }
    }
    if(c == 0){
        task[0]->counter = 1;
        init_test_case();
    }

    if(current->pid!=task[next]->pid)
    {
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n", 
        current->pid,task[next]->pid,
        current->pid, (unsigned long)current->thread.sp, 
        task[next]->pid, (unsigned long)task[next], 
        task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif

#ifdef PRIORITY

//simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void) {
    if (!task_init_done) return;
    if (task_test_done) return;
    printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter,current->priority);
    //current process's counter -1, judge whether to schedule or go on.
    /*your code*/
    current->counter --;
    schedule();
}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    unsigned char next; 
    /*your code*/
    int maxPriority = task[NR_TASKS-1]->priority ;
    int remainShortest = task[NR_TASKS-1]->counter;
    char flag = 2 ;
    for (int i = NR_TASKS-2; i >= 0; i--){
        /* code */
        if( task[i]->priority > maxPriority ){
            next = i ; 
        }
        if ( task[i]->priority == maxPriority){
            if( task[i]->counter < remainShortest  ){
                next = i ;// if they have equal priority, don't change 'next' since we select by iteration order.
            }
        }
        if (task[i]->counter > 0 ){
            flag = 1;
        }
    }
    if (flag == 2){
        // allocate a time slice to task0
        init_test_case();// I wonder whether we can call it directly, we do not declare it ?
    }
    if(current->pid != task[next]->pid)
    {
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n", 
        current->pid,task[next]->pid,
        current->pid, (unsigned long)current->thread.sp, 
        task[next]->pid, (unsigned long)task[next], 
        task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif



