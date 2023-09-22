#include "all.h"

struct s
token_kind_name(enum token_kind k)
{
	switch (k) {
	case T_INVALID:
		return sstr("INVALID");
	case T_IDENT:
		return sstr("IDENT");
	case T_NUMBER:
		return sstr("NUMBER");
	}
}

static bool
whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool
number(char c)
{
	return c >= '0' && c <= '9';
}

static bool
ident_start(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool
ident_follow(char c)
{
	return ident_start(c) || number(c) || (c == '_');
}

void
lex(struct tokens *t, struct mem *m, struct s input)
{
	struct arena_temp temp = arena_temp_begin(&m->temp);

	// the maximum number of tokens is one per byte of input
	enum token_kind *kinds = alloc(&m->temp, enum token_kind, input.n);
	struct span *spans = alloc(&m->temp, struct span, input.n);
	usize count = 0;

	char *inp = (char *)input.p;
	u32 i = 0;
	while (i < input.n) {
		if (whitespace(inp[i])) {
			while (i < input.n && whitespace(inp[i])) {
				i++;
			}
			continue;
		}

		if (ident_start(inp[i])) {
			u32 start = i;
			while (i < input.n && ident_follow(inp[i])) {
				i++;
			}
			kinds[count] = T_IDENT;
			spans[count] =
			        (struct span){ .start = start, .end = i };
			count++;
			continue;
		}

		if (number(inp[i])) {
			u32 start = i;
			while (i < input.n && number(inp[i])) {
				i++;
			}
			kinds[count] = T_NUMBER;
			spans[count] =
			        (struct span){ .start = start, .end = i };
			count++;
			continue;
		}

		kinds[count] = T_INVALID;
		spans[count] = (struct span){ .start = i, .end = i + 1 };
		i++;
		count++;
		continue;
	}

	t->kinds = alloc_copy(&m->perm, enum token_kind, kinds, count);
	t->spans = alloc_copy(&m->perm, struct span, spans, count);
	t->count = count;

	arena_temp_end(temp);
}

struct s
lex_test(struct mem *m, struct s input)
{
	struct tokens toks = { 0 };
	lex(&toks, m, input);

	struct arena_temp t = arena_temp_begin(&m->temp);

	struct arena output = { 0 };
	alloc_arena(&m->temp, &output, 1024 * 1024, 1);

	for (usize i = 0; i < toks.count; i++) {
		enum token_kind kind = toks.kinds[i];
		struct span span = toks.spans[i];
		struct s name = token_kind_name(kind);
		arena_printf(&output, "%.*s@%d..%d\n", (int)name.n, name.p,
		        span.start, span.end);
	}

	void *p = alloc_copy_arena(&m->perm, &output, 1);
	arena_temp_end(t);
	return create_s(p, output.used);
}
