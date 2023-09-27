#include "all.h"

void
strbuilder_init(struct strbuilder *sb, struct str buf)
{
	sb->buf = buf;
	sb->used = 0;
}

struct str
strbuilder_done(struct strbuilder *sb)
{
	return str_make(sb->buf.p, sb->used);
}

void
strbuilder_byte(struct strbuilder *sb, u8 b)
{
	usize remaining = sb->buf.n - sb->used;
	assert(1 <= remaining);
	sb->buf.p[sb->used] = b;
	sb->used++;
}

void
strbuilder_push(struct strbuilder *sb, struct str s)
{
	usize remaining = sb->buf.n - sb->used;
	assert(s.n <= remaining);
	memcpy(sb->buf.p + sb->used, s.p, s.n);
	sb->used += s.n;
}

void
strbuilder_printf(struct strbuilder *sb, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	usize remaining = sb->buf.n - sb->used;

	// vsnprintf returns the number of bytes it would have written,
	// were the buffer we pass it of unlimited size.
	usize len = cast(usize) vsnprintf(
	        cast(char *) sb->buf.p + sb->used, remaining, fmt, args);
	assert(len <= remaining);
	sb->used += len;

	va_end(args);
}
