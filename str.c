#include "all.h"

struct str
str_make(void *p, usize n)
{
	struct str s;
	s.p = p;
	s.n = n;
	return s;
}
