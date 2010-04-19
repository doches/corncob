#include "ct_hash_test.h"

int count = 0;
void print_pair(hash_element *element)
{
	//printf("%d:\t%d\n",element->key,element->value);
	count++;
}

void test_hash()
{
	// Create a hash and populate it
	ct_hash *map = hash_new(10);
	assert(map->size == 0);
	
	for (int i=0; i<20; i++) {
		hash_add(map, i, 100+i);
	}
	
	// Test size
	assert(map->size == 20);
	
	// Test get with valid key
	hash_element *e = hash_get(map, 5);
	assert(e->value == 105);
	
	// Test get with invalid key
	e = hash_get(map, -1);
	assert(e->key==-1);
	
	// Test get-and-modify
	e = hash_get(map, 0);
	e->value = 42;
	assert(e->value == 42);
	e = hash_get(map, 0);
	assert(e->value == 42);
	
	// Test overwrite
	e = hash_get(map, 0);
	assert(e->value == 42);
	hash_add(map, 0, 12);
	e = hash_get(map, 0);
	assert(e->value == 12);
	
	// Test iteration
	count = 0;
	hash_foreach(map, &print_pair);
	assert(count == 20);
	
	hash_free(map);
}

