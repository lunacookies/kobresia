#include "all.h"

enum { MAX_DIAGNOSTICS = 1024 };

struct diagnostics_store
create_diagnostics_store(struct mem *m)
{
	return (struct diagnostics_store){
		.sev = alloc(&m->perm, enum severity, MAX_DIAGNOSTICS),
		.msg = alloc(&m->perm, char, 128 * MAX_DIAGNOSTICS),
		.msglen = alloc(&m->perm, u32, MAX_DIAGNOSTICS),
		.count = 0,
	};
}
