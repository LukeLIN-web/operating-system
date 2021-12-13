#include "test.h"
#include "sched.h"
#include "slub.h"
#define SIFIVE_TEST 0x100000
#define VIRT_TEST_FINISHER_PASS 0x5555

int start_kernel()
{
	slub_init();
	// task_init();
	os_test();

	asm volatile("sh %0, 0(%1)" : : "r" (VIRT_TEST_FINISHER_PASS), "r" (SIFIVE_TEST));

	return 0;
}
