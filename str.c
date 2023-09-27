#include "all.h"

struct str
str_make(void *p, usize n)
{
	return cast(struct str){
		.p = p,
		.n = n,
	};
}
