#ifndef COUNT_LIST_H
#define COUNT_LIST_H

#include "SparseCounts.h"
#include <stdlib.h>

#define SPARSECOUNTS_BUCKETS 256
#define COUNT_LIST_DEFAULT_SIZE 32

typedef struct count_list_t
{
	SparseCounts **counts;
	unsigned int size;
	unsigned int used;
} count_list;

count_list *count_list_new();
void count_list_free(count_list *);
SparseCounts *count_list_get(count_list *,unsigned int);
void count_list_add(count_list *);
void count_list_set(count_list *,SparseCounts *, unsigned int index);

#endif
