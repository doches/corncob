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
    assert(map->sum == 0);
	
	hash_element *a;
	for (int i=0; i<20; i++) {
		a = hash_add(map, i, 100+i);
		assert(a->key == i);
		assert(a->value == 100+i);
	}
	
	// Test size
	assert(map->size == 20);
	
	// Test get with valid key
	hash_element *e = hash_get(map, 5);
	assert(e->value == 105);
	
	// Test get with invalid key
	e = hash_get(map, -1);
	assert(e == NULL);
	
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
	
	// Test update
	  // ..with existing key
	e = hash_update(map,10,-1);
	assert(e->value == 109);
	assert(e->key == 10);
	  // ..with new key
	e = hash_get(map,312);
	assert(e == NULL);
	e = hash_update(map,312,9);
	assert(e->key == 312);
	assert(e->value == 9);
	
	
	// Test iteration
	count = 0;
	hash_foreach(map, &print_pair);
	assert(count == 21);
    
    // Clean up
	hash_free(map);
    
    // Test dot
    ct_hash *hash_a = hash_new(10);
    hash_add(hash_a, 0, 1);
    hash_add(hash_a, 1, 2);
    ct_hash *hash_b = hash_new(10);
    hash_add(hash_b, 1, 1);
    hash_add(hash_b, 2, 5);
    assert(hash_dot(hash_a, hash_b) == 2);
    hash_free(hash_a);
    hash_free(hash_b);
    
    // Test sum
    ct_hash *sum = hash_new(10);
        // Hash sum should start at zero
    assert(sum->sum == 0);
        // If we add a new item, sum should be its value
    hash_add(sum, 0, 5);
    assert(sum->sum == 5);
        // If we update that item, make sure the sum reflects the change
    hash_update(sum, 0, 3);
    assert(sum->sum == 8);
        // How 'bout negative updates?
    hash_update(sum, 0, -2);
    assert(sum->sum == 6);
        // If we add a key that already exists, make sure we subtracted the old value
    hash_add(sum,0,3);
    assert(sum->sum == 3);
        // Multiple values OK?
    hash_add(sum, 1, 1);
    assert(sum->sum == 4);
    hash_update(sum, 1, -1);
    assert(sum->sum == 3);
    hash_add(sum, 1, 5);
    assert(sum->sum == 8);
    hash_free(sum);
}

