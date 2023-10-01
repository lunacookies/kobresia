#include "all.h"

#define UNINIT_SENTINEL '*'
#define REUSED_SENTINEL '^'

void
arena_init(struct arena *a, struct str buf)
{
	assert_zero(a);
	a->buf = buf;
}

static struct str
alloc_uninit(struct arena *a, usize size, usize align)
{
	usize padding = align - (cast(usize)(a->buf.p + a->used) % align);
	if (padding == align) {
		padding = 0;
	}

	assert(a->buf.n - a->used >= size + padding);

	a->used += padding;
	struct str s = str_slice(a->buf, a->used, a->used + size);
	a->used += size;
	if (a->used > a->peak_used) {
		a->peak_used = a->used;
	}

	return s;
}

struct str
alloc_str(struct arena *a, usize size, usize align)
{
	usize peak_used = a->peak_used;
	struct str s = alloc_uninit(a, size, align);
	usize allocation_start = cast(usize)(s.p - a->buf.p);
	usize allocation_end = allocation_start + size - 1;

	if (allocation_end < peak_used) {
		// In this case the entirety of the allocation is reused memory,
		// so we zero out the whole thing.
#if DEVELOP
		assert(str_all(s, REUSED_SENTINEL));
#endif
		str_zero(s);
	} else if (peak_used <= allocation_start) {
		// In this case the entirety of the allocation
		// is in untouched memory,
		// so we don’t have to do anything to zero it out.
	} else {
		// In this case a portion of the allocation is reused memory,
		// so we zero out the reused portion.
		assert(peak_used > allocation_start);
		struct str reused =
		        str_slice(s, 0, peak_used - allocation_start);
#if DEVELOP
		assert(str_all(s, REUSED_SENTINEL)); // FIXME
#endif
		str_zero(reused);
	}

#if DEVELOP
	assert(str_all(s, 0));
#endif

	return s;
}

struct str
alloc_str_u(struct arena *a, usize size, usize align)
{
	struct str s = alloc_uninit(a, size, align);
#if DEVELOP
	str_fill(s, UNINIT_SENTINEL);
#endif
	return s;
}

struct str
alloc_copy_str(struct arena *a, struct str src, usize align)
{
	struct str dst = alloc_uninit(a, src.n, align);
	str_copy(dst, src);
	return dst;
}

void *
_alloc_copy(struct arena *a, void *data, usize size, usize align)
{
	struct str dst = alloc_uninit(a, size, align);
	struct str src = str_make(data, size);
	str_copy(dst, src);
	return dst.p;
}

struct arena_temp
arena_temp_begin(struct arena *a)
{
	a->temp_count++;
	struct arena_temp t;
	t.a = a;
	t.used = a->used;
	return t;
}

void
arena_temp_end(struct arena_temp t)
{
	assert(t.a->temp_count > 0);
	t.a->temp_count--;

	assert(t.used <= t.a->used);
	assert(t.used <= t.a->buf.n);
#if DEVELOP
	struct str added_during_temp = str_slice(t.a->buf, t.used, t.a->used);
	str_fill(added_during_temp, REUSED_SENTINEL);
#endif

	t.a->used = t.used;
}
