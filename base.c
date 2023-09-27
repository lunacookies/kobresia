#include "all.h"

void
_assert_zero(void *p, usize n)
{
	u8 *p_ = cast(u8 *) p;
	for (usize i = 0; i < n; i++) {
		assert(p_[i] == 0);
	}
}
