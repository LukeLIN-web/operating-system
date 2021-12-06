#include "put.h"
#include "sched.h"

int start_kernel(){
	const char *msg = "ZJU OS LAB 4     Student1:3180103721 林炬乙 Student2: 3190105134 乔一帆\n";
	puts(msg);
	task_init();
	dead_loop();

	return 0;
}
