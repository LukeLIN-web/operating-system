#include"put.h"
#include"sched.h"
#define offset (0xffffffe000000000-0x80000000)

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0,1,2,3,4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0,4,3,2,1};

extern void init_epc(void);
extern void __switch_to(struct task_struct* current, struct task_struct* next);
extern unsigned int rand();
extern uint64_t cur;


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
        tmp->counter = COUNTER_INIT_COUNTER[i];
        tmp->priority = PRIORITY_INIT_COUNTER[i];
        tmp->blocked = 0;
        tmp->pid = i;
        task[i] = tmp;
        task[i]->thread.sp = (unsigned long long) task[i] + TASK_SIZE; 
        task[i]->thread.ra = &__init_sepc;
        printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
    }
    task_init_done = 1;
}

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


void switch_to(struct task_struct* next)
{
	
}

void dead_loop(void)
{
	while (1) { continue; }
}

