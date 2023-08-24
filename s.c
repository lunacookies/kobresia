#include "all.h"

struct s
create_s(u8 *p, usize n)
{
	return (struct s){
		.p = p,
		.n = 0,
		.total = n,
	};
}

struct s
create_s_full(u8 *p, usize n)
{
	return (struct s){
		.p = p,
		.n = n,
		.total = n,
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
	memset(p, '*', size);
	return p;
}

struct s
alloc_s(struct s *s, usize size)
{
	void *p = _alloc(s, size, 1);
	struct s new = create_s(p, size);
	return new;
}

void
alloc_copy(struct s *s, void *data, usize size, usize align)
{
	u8 *p = _alloc(s, size, align);
	memcpy(p, data, size);
}
