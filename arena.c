#include "all.h"

void
init_arena(struct arena *a, struct s buf)
{
	a->buf = buf;
	a->used = 0;
	a->temp_count = 0;
}

void *
_alloc(struct arena *a, usize size, usize align)
{
	usize padding = align - ((usize)(a->buf.p + a->used) % align);
	if (padding == align) {
		padding = 0;
	}

	assert(a->buf.n - a->used >= size + padding);

	a->used += padding;
	void *p = a->buf.p + a->used;
	a->used += size;

#if DEVELOP
	memset(p, '*', size);
#endif

	return p;
}

struct s
alloc_s(struct arena *a, usize size, usize align)
{
	void *p = _alloc(a, size, align);
	struct s new = create_s(p, size);
	return new;
}

void
alloc_arena(struct arena *a, struct arena *out, usize size, usize align)
{
	struct s buf = alloc_s(a, size, align);
	init_arena(out, buf);
}

void *
_alloc_copy(struct arena *a, void *data, usize size, usize align)
{
	u8 *p = _alloc(a, size, align);
	memcpy(p, data, size);
	return p;
}

void *
alloc_copy_s(struct arena *a, struct s from, usize align)
{
	u8 *p = _alloc(a, from.n, align);
	memcpy(p, from.p, from.n);
	return p;
}

void *
alloc_copy_arena(struct arena *a, struct arena *from, usize align)
{
	struct s used = create_s(from->buf.p, from->used);
	return alloc_copy_s(a, used, align);
}

struct arena_temp
arena_temp_begin(struct arena *a)
{
	a->temp_count++;
	return (struct arena_temp){
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

void
arena_printf(struct arena *a, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	usize remaining = a->buf.n - a->used;
	usize written = (usize)vsnprintf(
	        (char *)a->buf.p + a->used, remaining, fmt, args);
	assert(written < remaining);
	a->used += written;
	va_end(args);
}
