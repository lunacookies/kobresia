#include "all.h"

struct s
create_s(u8 *p, usize n)
{
	return (struct s){
		.p = p,
		.n = 0,
		.total = n,
		.temp_count = 0,
	};
}

struct s
create_s_full(u8 *p, usize n)
{
	return (struct s){
		.p = p,
		.n = n,
		.total = n,
		.temp_count = 0,
	};
}

void *
_alloc(struct s *s, usize size, usize align)
{
	usize padding = align - ((usize)(s->p + s->n) % align);
	if (padding == align) {
		padding = 0;
	}

	assert(s->total - s->n >= size + padding);

	s->n += padding;
	void *p = s->p + s->n;
	s->n += size;

#if DEVELOP
	memset(p, '*', size);
#endif

	return p;
}

struct s
alloc_s(struct s *s, usize size, usize align)
{
	void *p = _alloc(s, size, align);
	struct s new = create_s(p, size);
	return new;
}

void *
_alloc_copy(struct s *s, void *data, usize size, usize align)
{
	u8 *p = _alloc(s, size, align);
	memcpy(p, data, size);
	return p;
}

void *
alloc_copy_s(struct s *s, struct s from, usize align)
{
	u8 *p = _alloc(s, from.n, align);
	memcpy(p, from.p, from.n);
	return p;
}

struct s_temp
s_temp_begin(struct s *s)
{
	s->temp_count++;
	return (struct s_temp){
		.s = s,
		.n = s->n,
	};
}

void
s_temp_end(struct s_temp t)
{
	assert(t.s->temp_count > 0);
	t.s->temp_count--;
	t.s->n = t.n;
}

void
s_printf(struct s *s, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	usize remaining = s->total - s->n;
	usize written =
	        (usize)vsnprintf((char *)s->p + s->n, remaining, fmt, args);
	assert(written < remaining);
	s->n += written;
	va_end(args);
}
