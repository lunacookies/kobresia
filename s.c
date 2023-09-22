#include "all.h"

struct s
create_s(void *p, usize n)
{
	return (struct s){
		.p = p,
		.n = n,
	};
}
