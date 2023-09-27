#include "all.h"

void
arena_init(struct arena *a, struct str buf)
{
	a->buf = buf;
	a->used = 0;
	a->temp_count = 0;
}

static void *
alloc_uninit(struct arena *a, usize size, usize align)
{
	usize padding = align - (cast(usize)(a->buf.p + a->used) % align);
	if (padding == align) {
		padding = 0;
	}

	assert(a->buf.n - a->used >= size + padding);

	a->used += padding;
	void *p = a->buf.p + a->used;
	a->used += size;
	return p;
}

void *
_alloc(struct arena *a, usize size, usize align)
{
	void *p = alloc_uninit(a, size, align);
	memset(p, 0, size);
	return p;
}

void *
_alloc_u(struct arena *a, usize size, usize align)
{
	void *p = alloc_uninit(a, size, align);
#if DEVELOP
	memset(p, '*', size);
#endif
	return p;
}

struct str
alloc_str(struct arena *a, usize size, usize align)
{
	void *p = _alloc(a, size, align);
	struct str new = str_make(p, size);
	return new;
}

struct str
alloc_str_u(struct arena *a, usize size, usize align)
{
	void *p = _alloc_u(a, size, align);
	struct str new = str_make(p, size);
	return new;
}

void
alloc_arena(struct arena *a, struct arena *out, usize size)
{
	void *p = alloc_uninit(a, size, 1);
	struct str s = str_make(p, size);
	arena_init(out, s);
}

void *
_alloc_copy(struct arena *a, void *data, usize size, usize align)
{
	u8 *p = alloc_uninit(a, size, align);
	memcpy(p, data, size);
	return p;
}

struct str
alloc_copy_str(struct arena *a, struct str from, usize align)
{
	u8 *p = alloc_uninit(a, from.n, align);
	memcpy(p, from.p, from.n);
	return str_make(p, from.n);
}

struct arena_temp
arena_temp_begin(struct arena *a)
{
	a->temp_count++;
	return cast(struct arena_temp){
		.a = a,
		.used = a->used,
	};
}

void
arena_temp_end(struct arena_temp t)
{
	assert(t.a->temp_count > 0);
	t.a->temp_count--;
	t.a->used = t.used;
}
