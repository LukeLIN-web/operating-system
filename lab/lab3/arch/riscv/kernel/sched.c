#include "stdio.h"
#include "sched.h"

#define Kernel_Page            0x80210000
#define LOW_MEMORY             0x80211000
#define PAGE_SIZE              4096UL
 

struct task_struct* current;
struct task_struct* task[NR_TASKS];


extern task_test_done;
extern void __init_sepc(void);

//If next ==  current,do nothing; else update current and call __switch_to.
void switch_to(struct task_struct* next) {
    /*your code*/
    if(next->pid != current->pid){
        struct task_struct* prev = current;
        current = next;
        __switch_to(prev,current);
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
        struct task_struct* tmp = (struct task_struct*)(Kernel_Page+PAGE_SIZE*i);
        tmp->state = TASK_RUNNING;
        tmp->counter = 0;
        tmp->priority = 5;
        tmp->blocked = 0;
        tmp->pid = i;
        task[i] = tmp;
        task[i]->thread.sp = (unsigned long long) task[i] + TASK_SIZE; 
        task[i]->thread.ra = &__init_sepc;
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
    if(current->counter == 0)
        schedule();
}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    int next = -1; 
    long  min = 999;
    for(int i = LAB_TEST_NUM ;i >= 1;i --){
        if(task[i]->counter == 0)
            continue;
        if(task[i]->counter < min){
            next = i;
            min = task[next]->counter;
        }
    }
    if(next == -1){
        next = 0;
        task[0]->counter = 1;
    }
    if(next != -1 && current->pid != task[next]->pid){
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
    if(current->counter == 0)
        schedule();
}

//Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would 
//assign new test case.
void schedule(void) {
    unsigned char next = -1; 
    /*your code*/
    int maxPriority = -1;
    int remainShortest = 999;
    char flag = 2 ;
    for (int i = LAB_TEST_NUM; i >= 1; i--){
        /* code */
        if(task[i]->counter <= 0)
            continue;
        if( task[i]->priority > maxPriority ){
            next = i ; 
            maxPriority = task[i]->priority;
            // remainShortest = task[i]->counter;
        }
        else{
            if ( task[i]->priority == maxPriority){
                if( task[i]->counter < remainShortest ){
                    next = i ;// if they have equal priority, don't change 'next' since we select by iteration order.
                    remainShortest = task[i]->counter;
                }
            }
        }
        if (task[i]->counter > 0)
            flag = 1;
    }
    if (flag == 2  ){
        // allocate a time slice to task0
        task[0]->counter = 1;
        //init_test_case();// I wonder whether we can call it directly, we do not declare it ?
        next = 0;
    }
    if(current->pid != task[next]->pid){
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n", 
        current->pid,task[next]->pid,
        current->pid, (unsigned long)current->thread.sp, 
        task[next]->pid, (unsigned long)task[next], 
        task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif



