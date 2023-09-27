#include "all.h"

#define UNINIT_SENTINEL '*'
#define REUSED_SENTINEL '^'

void
arena_init(struct arena *a, struct str buf)
{
	assert_zero(a);
	a->buf = buf;
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
	if (a->used > a->peak_used) {
		a->peak_used = a->used;
	}

	return p;
}

void *
_alloc(struct arena *a, usize size, usize align)
{
	usize peak_used = a->peak_used;
	u8 *p = alloc_uninit(a, size, align);
	usize allocation_start = cast(usize)(p - a->buf.p);
	usize allocation_end = allocation_start + size - 1;

	if (allocation_end < peak_used) {
		// In this case the entirety of the allocation is reused memory,
		// so we zero out the whole thing.
#if DEVELOP
		for (usize i = 0; i < size; i++) {
			assert(p[i] == REUSED_SENTINEL);
		}
#endif
		memset(p, 0, size);
	} else if (peak_used <= allocation_start) {
		// In this case the entirety of the allocation
		// is in untouched memory,
		// so we don’t have to do anything to zero it out.
	} else {
		// In this case a portion of the allocation is reused memory,
		// so we zero out the reused portion.
		assert(peak_used > allocation_start);
		usize reused_size = peak_used - allocation_start;
#if DEVELOP
		for (usize i = 0; i < reused_size; i++) {
			assert(p[i] == REUSED_SENTINEL);
		}
#endif
		memset(p, 0, reused_size);
	}

#if DEVELOP
	for (usize i = 0; i < size; i++) {
		assert(p[i] == 0);
	}
#endif

	return p;
}

void *
_alloc_u(struct arena *a, usize size, usize align)
{
	void *p = alloc_uninit(a, size, align);
#if DEVELOP
	memset(p, UNINIT_SENTINEL, size);
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
	usize peak_used = a->peak_used;
	u8 *p = alloc_uninit(a, size, 1);
	usize new_used = a->used;

	// Sub-arenas must always be created on memory
	// which has never been touched before, i.e. is still zeroed from mmap.
	assert(peak_used <= new_used);

#if DEVELOP
	// Check the first little bit of sub-arena memory
	// to make sure we really are on an untouched area of memory.

	usize check_length = 128;
	if (check_length > size) {
		check_length = size;
	}

	for (usize i = 0; i < check_length; i++) {
		assert(p[i] == 0);
	}
#endif

	arena_init(out, str_make(p, size));
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
	usize amount_added_during_temp = t.a->used - t.used;
	memset(t.a->buf.p + t.used, REUSED_SENTINEL, amount_added_during_temp);
#endif

	t.a->used = t.used;
}
