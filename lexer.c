#include "all.h"

struct str
token_kind_name(enum token_kind k)
{
	switch (k) {
	case T_INVALID:
		return S("INVALID");
	case T_IDENT:
		return S("IDENT");
	case T_NUMBER:
		return S("NUMBER");
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
lex(struct tokens *t, struct mem *m, struct str input)
{
	assert_zero(t);

	struct arena_temp temp = arena_temp_begin(&m->temp);

	// the maximum number of tokens is one per byte of input
	enum token_kind *kinds = alloc_u(&m->temp, enum token_kind, input.n);
	struct span *spans = alloc_u(&m->temp, struct span, input.n);
	imm count = 0;

	char *inp = cast(char *) input.p;
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

			zero_out(&spans[count]);
			spans[count].start = start;
			spans[count].end = i;

			count++;
			continue;
		}

		if (number(inp[i])) {
			u32 start = i;
			while (i < input.n && number(inp[i])) {
				i++;
			}

			kinds[count] = T_NUMBER;

			zero_out(&spans[count]);
			spans[count].start = start;
			spans[count].end = i;

			count++;
			continue;
		}

		kinds[count] = T_INVALID;

		zero_out(&spans[count]);
		spans[count].start = i;
		spans[count].end = i + 1;

		i++;
		count++;
		continue;
	}

	t->kinds = alloc_copy(&m->perm, enum token_kind, kinds, count);
	t->spans = alloc_copy(&m->perm, struct span, spans, count);
	t->count = count;

	arena_temp_end(temp);
}

struct str
lex_test(struct mem *m, struct str input)
{
	struct tokens toks;
	lex(&toks, m, input);

	struct arena_temp t = arena_temp_begin(&m->temp);
	struct strbuilder out;
	strbuilder_init(&out, alloc_str_u(&m->temp, 16 * KIBIBYTE, 1));

	for (imm i = 0; i < toks.count; i++) {
		enum token_kind kind = toks.kinds[i];
		struct span span = toks.spans[i];
		struct str name = token_kind_name(kind);
		strbuilder_printf(&out, "%.*s@%d..%d\n", cast(int) name.n,
		        name.p, span.start, span.end);
	}

	struct str out_s = strbuilder_done(&out);
	out_s = alloc_copy_str(&m->perm, out_s, 1);
	arena_temp_end(t);
	return out_s;
}
