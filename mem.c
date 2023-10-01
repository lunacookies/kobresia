#include "all.h"

enum {
	PERM_SIZE = 1 * MEBIBYTE,
	TEMP_SIZE = 100 * MEBIBYTE,
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
	struct str b = *block;
	struct str perm_buf = str_prefix(b, PERM_SIZE);
	struct str temp_buf = str_slice(b, PERM_SIZE, PERM_SIZE + TEMP_SIZE);
	*block = str_suffix(b, PERM_SIZE + TEMP_SIZE);

	arena_init(&out->perm, perm_buf);
	arena_init(&out->temp, temp_buf);
}

void
proc_mem_alloc(struct proc_mem *pm, u32 core_count)
{
	assert_zero(pm);

	usize total = (core_count + 1) * (PERM_SIZE + TEMP_SIZE);
	struct str block = os_alloc(total);

	push_mem(&block, &pm->main);
	pm->workers = alloc(&pm->main.perm, struct mem, core_count);
	for (u32 i = 0; i < core_count; i++) {
		push_mem(&block, &pm->workers[i]);
	}

	assert(block.n == 0);
}
