#include "all.h"

struct s
create_s(u8 *p, usize n)
{
	return (struct s){ .top = p, .p = p, .n = n };
}

void
split_off(struct s *s, usize n, struct s *fst, struct s *snd)
{
	*fst = create_s(s->p, n);
	*snd = create_s(s->p + n, s->n - n);
	*s = create_s(NULL, 0);
}

void *
_alloc(struct s *s, usize size, usize align)
{
	usize padding = align - ((usize)s->p % align);
	if (padding == align) {
		padding = 0;
	}

	assert(s->n >= size + padding);

	s->p += padding;
	s->n -= padding;

	void *p = s->p;

	s->p += size;
	s->n -= size;

	return p;
}

struct s
alloc_s(struct s *s, usize size)
{
	assert(s->n >= size);
	struct s new = create_s(s->p, size);
	s->p += size;
	s->n -= size;
	return new;
}

void
alloc_copy(struct s *s, void *data, usize size, usize align)
{
	u8 *p = _alloc(s, size, align);
	memcpy(p, data, size);
}

usize
s_used(struct s *s)
{
	return s->p - s->top;
}
