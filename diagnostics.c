#include "all.h"

enum { MAX_DIAGNOSTICS = 1024 };

void
init_diagnostics_store(struct diagnostics_store *d, struct mem *m)
{
	d->sev = alloc(&m->perm, enum severity, MAX_DIAGNOSTICS);
	d->msg = alloc(&m->perm, char, 128 * MAX_DIAGNOSTICS);
	d->msglen = alloc(&m->perm, u32, MAX_DIAGNOSTICS);
	d->count = 0;
}
