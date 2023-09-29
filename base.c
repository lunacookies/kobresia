#include "all.h"

struct str
str_make(void *p, usize n)
{
	struct str s;
	s.p = p;
	s.n = n;
	return s;
}

void
_assert_zero(struct str s)
{
	u8 *p = cast(u8 *) s.p;
	for (usize i = 0; i < s.n; i++) {
		assert(p[i] == 0);
	}
}
