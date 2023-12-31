#include "all.h"

void
early_death(struct str msg)
{
	// we can’t even allocate memory,
	// so we do what little we can
	// and die a graceful death.
	write(STDERR_FILENO, "fatal error: ", 13);
	write(STDERR_FILENO, msg.p, cast(umm) msg.n);
	write(STDERR_FILENO, "\n", 1);
	exit(1);
}
