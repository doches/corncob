#ifndef COUNT_LIST_H
#define COUNT_LIST_H

#include "SparseCounts.h"
#include <stdlib.h>

#define COUNT_LIST_BUCKETS 400

typedef struct count_list_t
{
	SparseCounts *counts;
	struct count_list_t *next;
} count_list;

count_list *count_list_new();
void count_list_free(count_list *);
SparseCounts *count_list_get(count_list *,unsigned int);

#endif
