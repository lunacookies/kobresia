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
		early_death(sstr("failed to get page size"));
	}

	return (usize)s;
}

struct s
os_alloc(usize nbytes)
{
	assert(nbytes % page_size() == 0);

	void *p = mmap(NULL, nbytes, PROT_READ | PROT_WRITE,
	        MAP_ANON | MAP_PRIVATE, -1, 0);

	if (p == MAP_FAILED) {
		early_death(sstr("failed to get memory from the OS"));
	}

	return create_s(p, nbytes);
}

struct proc_mem
alloc_proc_mem(u32 core_count)
{
	usize total = (core_count + 1) * (PERM_MEM_SIZE + TEMP_MEM_SIZE);
	struct s block = os_alloc(total);

	struct mem main = {
		.perm = alloc_s(&block, PERM_MEM_SIZE),
		.temp = alloc_s(&block, TEMP_MEM_SIZE),
	};

	struct mem *workers = alloc(&main.perm, struct mem, core_count);

	for (u32 i = 0; i < core_count; i++) {
		workers[i] = (struct mem){
			.perm = alloc_s(&block, PERM_MEM_SIZE),
			.temp = alloc_s(&block, TEMP_MEM_SIZE),
		};
	}

	assert(block.n == block.total);

	return (struct proc_mem){ .main = main, .workers = workers };
}
