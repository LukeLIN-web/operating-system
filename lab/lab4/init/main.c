#include "put.h"
#include "sched.h"

int start_kernel()
{
	const char *msg = "ZJU OS LAB 4     Student1:123456 张三 Student2:123456 李四\n";
	puts(msg);
	task_init();
	dead_loop();

	return 0;
}
