#include "all.h"

enum { MAX_DIAGNOSTICS = 1024 };

void
diagnostics_store_init(struct diagnostics_store *d, struct mem *m)
{
	assert_zero(d);
	d->sev = alloc_u(&m->perm, enum severity, MAX_DIAGNOSTICS);
	d->msg = alloc_u(&m->perm, char, 128 * MAX_DIAGNOSTICS);
	d->msglen = alloc_u(&m->perm, s32, MAX_DIAGNOSTICS);
}
