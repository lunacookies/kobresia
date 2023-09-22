#include "all.h"

enum {
	PERM_MEM_SIZE = 1 * MEBIBYTE,
	TEMP_MEM_SIZE = 100 * MEBIBYTE,
};

static usize
page_size(void)
{
	i32 s = getpagesize();

	if (s == -1) {
		early_death(S("failed to get page size"));
	}

	return (usize)s;
}

struct str
os_alloc(usize nbytes)
{
	assert(nbytes % page_size() == 0);

	void *p = mmap(NULL, nbytes, PROT_READ | PROT_WRITE,
	        MAP_ANON | MAP_PRIVATE, -1, 0);

	if (p == MAP_FAILED) {
		early_death(S("failed to get memory from the OS"));
	}

	return str_make(p, nbytes);
}

void
proc_mem_alloc(struct proc_mem *pm, u32 core_count)
{
	usize ps = page_size();

	usize total = (core_count + 1) * (PERM_MEM_SIZE + TEMP_MEM_SIZE);
	struct arena block = { 0 };
	arena_init(&block, os_alloc(total));

	alloc_arena(&block, &pm->main.perm, PERM_MEM_SIZE, ps);
	alloc_arena(&block, &pm->main.temp, TEMP_MEM_SIZE, ps);

	pm->workers = alloc(&pm->main.perm, struct mem, core_count);

	for (u32 i = 0; i < core_count; i++) {
		alloc_arena(&block, &pm->workers[i].perm, PERM_MEM_SIZE, ps);
		alloc_arena(&block, &pm->workers[i].temp, TEMP_MEM_SIZE, ps);
	}

	assert(block.used == block.buf.n);
}
