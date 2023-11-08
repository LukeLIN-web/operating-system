#include "sched.h";
volatile int task_test_done=0;
extern printf();
extern ticks; 

//该变量供test0更改任务的时间片与优先级，同学们可以进行修改，自己的算法运行是否符合预期
//建议多检测些边界条件，例如优先级时间片均相同的情况。
// int counter_priority[16][2]={
//                             {3,1},{4,1},{1,3},{2,1},
//                             {1,1},{1,1},{1,1},{2,1},
//                             {5,1},{1,1},{1,2},{2,4},
//                             {5,1},{1,1},{0,1},{2,1}
//                             };
int counter_priority[16][2]={
                            {1,5},{1,6},{1,4},{1,2},
                            {1,5},{1,4},{3,4},{1,2},
                            {1,5},{1,4},{1,4},{1,2},
                            {2,5},{2,1},{2,1},{0,1}
                            };

/*该函数用于批改系统对同学们的代码进行批改，同学们也可修改该文件中的任意代码改变输出进行自测。
批改时，整个test.c文件会被替换。*/
int test(){
    switch (current->pid%5){
    case 0:
        printf("initialize test case....\n");
        init_test_case();
        break;
    default:
        dead_loop();
        break;
    }
    return 0;
}

void dead_loop(){
    while(1){}
}

void init_test_case(){
    int c_p_i = 0;
    int flag = 1;
    while(1){ 
        if(ticks % 10 == 5 ){
            if(flag == 1){//ticks%10与flag用于确保每个时间片只执行一次下面的代码片段
                if(c_p_i == 16){
                    printf("\n**********************************************************\nNo remaining test case, please enter ctrl+a+x to quit. \n"); 
                    while(1){
                        task_test_done = 1;
                    }
                }
                printf("\n**********************************************************\nTask0 is running to allocate new priority and counter.\n"); 
                for(int i = 1;i <= 4;i++){//对四个task进行时间片与优先级的重新划分
                   task[i]->counter = counter_priority[c_p_i%16][0];
                   task[i]->priority = counter_priority[c_p_i%16][1];
                   c_p_i++;
                }
            }
            flag = 0;
        }
        else flag = 1;
    }
}
