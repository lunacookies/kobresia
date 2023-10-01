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
str_copy(struct str dst, struct str src)
{
	assert(dst.n == src.n);
	memcpy(dst.p, src.p, dst.n);
}

void
str_fill(struct str s, u8 b)
{
	memset(s.p, b, s.n);
}

void
str_zero(struct str s)
{
	str_fill(s, 0);
}

struct str
str_slice(struct str s, usize start, usize end)
{
	assert(start <= s.n);
	assert(end <= s.n);
	return str_make(s.p + start, end - start);
}

bool
str_equal(struct str s1, struct str s2)
{
	if (s1.n != s2.n) {
		return false;
	}

	if (s1.p == s2.p) {
		return true;
	}

	return memcmp(s1.p, s2.p, s1.n) == 0;
}

bool
str_all(struct str s, u8 b)
{
	u8 *end = s.p + s.n;

	for (u8 *cur = s.p; cur < end; cur++) {
		if (*cur != b) {
			return false;
		}
	}

	return true;
}

void
_assert_zero(struct str s)
{
	u8 *p = cast(u8 *) s.p;
	for (usize i = 0; i < s.n; i++) {
		assert(p[i] == 0);
	}
}
