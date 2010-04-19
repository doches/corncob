#ifndef SPARSE_COUNTS_H
#define SPARSE_COUNTS_H

#include <stdlib.h>
#include "ct_hash.h"

typedef struct t_SparseCounts {
  int total;
  ct_hash *map;
} SparseCounts;

SparseCounts *SparseCounts_new(unsigned int buckets);
hash_element *SparseCounts_add(SparseCounts *counts, int key, int value);
hash_element *SparseCounts_getElement(SparseCounts *counts, int key);
unsigned int SparseCounts_getValue(SparseCounts *counts, int key);
void SparseCounts_free(SparseCounts *counts);

#endif
