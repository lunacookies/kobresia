#include "all.h"

struct str
str_make(void *p, usize n)
{
	return (struct str){
		.p = p,
		.n = n,
	};
}
