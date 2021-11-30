#include "test.h"
#include "put.h"
#include "sched.h"

int os_test()
{
	const char *msg = "ZJU OS LAB              Guo Ziyang\n";

    puts(msg);
    task_init();
    // unsigned long rodata_start = 0;
    // asm("la t1, rodata_start\n");
    // asm("sd t2, 0(t1)\n");
    // asm("auipc t0, 0x0\n");

	while(1) ;

    return 0;
}
