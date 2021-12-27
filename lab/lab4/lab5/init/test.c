#include "test.h"
#include "put.h"
#include "sched.h"

int os_test()
{
	const char *msg = "ZJU OS LAB              Lin Juyi3180103721\n";

    puts(msg);
    task_init();

	while(1) {};

    return 0;
}
