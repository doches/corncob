#include "SparseCounts.h"

SparseCounts *SparseCounts_new(unsigned int buckets)
{
  SparseCounts *new = malloc(sizeof(SparseCounts));
  new->total = 0;
  new->map = hash_new(buckets);
  return new;
}

hash_element *SparseCounts_add(SparseCounts *counts, int key, int value)
{
  hash_element *element = SparseCounts_getElement(counts,key);
  if (element == NULL || element->key != key) { // key not found in map, so add it
  	counts->total += value;
    return hash_add(counts->map,key,value);
  } else { // Key found in map, so update it
    element->value += value;
  	counts->total += value;
  	return element;
  }
}

hash_element *SparseCounts_getElement(SparseCounts *counts, int key)
{
  hash_element *element = hash_get(counts->map, key);
  return element;
}

// Returns -1 if key is not found!
unsigned int SparseCounts_getValue(SparseCounts *counts, int key)
{
  hash_element *element = SparseCounts_getElement(counts,key);
  if(element != NULL) {
    return element->value;
  } else {
    return 0;
  }
}

void SparseCounts_free(SparseCounts *counts)
{
	hash_free(counts->map);
	free(counts);
}
