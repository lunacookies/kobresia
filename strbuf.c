#include "all.h"

void
strbuf_init(struct strbuf *sb, struct str buf)
{
	sb->buf = buf;
	sb->used = 0;
}

struct str
strbuf_done(struct strbuf *sb)
{
	return str_make(sb->buf.p, sb->used);
}

void
strbuf_byte(struct strbuf *sb, u8 b)
{
	usize remaining = sb->buf.n - sb->used;
	assert(1 <= remaining);
	sb->buf.p[sb->used] = b;
	sb->used++;
}

void
strbuf_push(struct strbuf *sb, struct str s)
{
	usize remaining = sb->buf.n - sb->used;
	assert(s.n <= remaining);
	memcpy(sb->buf.p + sb->used, s.p, s.n);
	sb->used += s.n;
}

void
strbuf_printf(struct strbuf *sb, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	usize remaining = sb->buf.n - sb->used;

	// vsnprintf returns the number of bytes it would have written,
	// were the buffer we pass it of unlimited size.
	usize len = (usize)vsnprintf(
	        (char *)sb->buf.p + sb->used, remaining, fmt, args);
	assert(len <= remaining);
	sb->used += len;

	va_end(args);
}
