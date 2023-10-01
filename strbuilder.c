#include "all.h"

void
strbuilder_init(struct strbuilder *sb, struct str buf)
{
	assert_zero(sb);
	sb->buf = buf;
}

struct str
strbuilder_done(struct strbuilder *sb)
{
	return str_slice(sb->buf, 0, sb->used);
}

static struct str
strbuilder_remaining(struct strbuilder *sb)
{
	return str_slice(sb->buf, sb->used, sb->buf.n);
}

void
strbuilder_byte(struct strbuilder *sb, u8 b)
{
	struct str remaining = strbuilder_remaining(sb);
	assert(remaining.n >= 1);
	remaining.p[0] = b;
	sb->used++;
}

void
strbuilder_push(struct strbuilder *sb, struct str s)
{
	struct str remaining = strbuilder_remaining(sb);
	str_copy(str_slice(remaining, 0, s.n), s);
	sb->used += s.n;
}

void
strbuilder_printf(struct strbuilder *sb, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	struct str remaining = strbuilder_remaining(sb);

	// vsnprintf returns the number of bytes it would have written,
	// were the buffer we pass it of unlimited size.
	imm len = cast(imm) vsnprintf(
	        cast(char *) remaining.p, cast(umm) remaining.n, fmt, args);
	assert(len <= remaining.n);
	sb->used += len;

	va_end(args);
}
