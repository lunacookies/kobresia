#include "all.h"

enum {
	PERM_SIZE = 1 * MEBIBYTE,
	TEMP_SIZE = 100 * MEBIBYTE,
};

static smm
page_size(void)
{
	s32 s = getpagesize();

	if (s == -1) {
		early_death(S("failed to get page size"));
	}

	return cast(smm) s;
}

struct str
os_alloc(smm nbytes)
{
	assert(nbytes > 0);
	assert(nbytes % page_size() == 0);

	void *p = mmap(NULL, cast(umm) nbytes, PROT_READ | PROT_WRITE,
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
	struct str perm_buf = str_slice(b, 0, PERM_SIZE);
	struct str temp_buf = str_slice(b, PERM_SIZE, PERM_SIZE + TEMP_SIZE);
	*block = str_slice(b, PERM_SIZE + TEMP_SIZE, b.n);

	arena_init(&out->perm, perm_buf);
	arena_init(&out->temp, temp_buf);
}

void
proc_mem_alloc(struct proc_mem *pm, smm core_count)
{
	assert_zero(pm);
	assert(core_count > 0);

	smm total = (core_count + 1) * (PERM_SIZE + TEMP_SIZE);
	struct str block = os_alloc(total);

	push_mem(&block, &pm->main);
	pm->workers = alloc(&pm->main.perm, struct mem, core_count);
	for (smm i = 0; i < core_count; i++) {
		push_mem(&block, &pm->workers[i]);
	}

	assert(block.n == 0);
}
