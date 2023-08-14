#include "all.h"

static void
work(u32 i, void *arg)
{
	printf("[%d] doing work with arg %p\n", i, arg);

	struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 };
	nanosleep(&ts, NULL);

	printf("[%d] done\n", i);
}

int
main(void)
{
	u32 c = core_count();
	struct proc_mem pm = alloc_proc_mem(c);
	struct diagnostics_store diagnostics_store =
	        create_diagnostics_store(&pm.main);

	struct pool *p = start_pool(&pm.main, c, QOS_CLASS_UTILITY);
	void **args = alloc(&pm.main.perm, void *, c);
	for (u32 i = 0; i < c; i++) {
		args[i] = (void *)(0xdeadbeef + i);
	}
	execute(p, work, args);
	execute(p, work, args);
}
