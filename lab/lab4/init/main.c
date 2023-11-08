#include "put.h"
#include "sched.h"

int start_kernel(){
	const char *msg = "ZJU OS LAB 4 \n";
	puts(msg);
	task_init();
	dead_loop();

	return 0;
}
