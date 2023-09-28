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

	return cast(usize) s;
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

static void
push_mem(struct str *block, struct mem *out)
{
	usize total = PERM_MEM_SIZE + TEMP_MEM_SIZE;
	assert(block->n >= total);

	struct str perm_buf = str_make(block->p, PERM_MEM_SIZE);
	struct str temp_buf = str_make(block->p + PERM_MEM_SIZE, TEMP_MEM_SIZE);
	*block = str_make(block->p + total, block->n - total);

	arena_init(&out->perm, perm_buf);
	arena_init(&out->temp, temp_buf);
}

void
proc_mem_alloc(struct proc_mem *pm, u32 core_count)
{
	assert_zero(pm);

	usize total = (core_count + 1) * (PERM_MEM_SIZE + TEMP_MEM_SIZE);
	struct str block = os_alloc(total);

	push_mem(&block, &pm->main);
	pm->workers = alloc(&pm->main.perm, struct mem, core_count);
	for (u32 i = 0; i < core_count; i++) {
		push_mem(&block, &pm->workers[i]);
	}

	assert(block.n == 0);
}
