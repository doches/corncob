#include "double_hash_test.h"

double double_count = 0;
void print_double_pair(double_hash_element *element)
{
	//printf("%d:\t%d\n",element->key,element->value);
	double_count++;
}

void test_double_hash()
{
	// Create a hash and populate it
	double_hash *map = double_hash_new(10);
	assert(map->size == 0);
    assert(map->sum == 0);
	
	double_hash_element *a;
	for (int i=0; i<20; i++) {
		a = double_hash_add(map, i, 100+i);
		assert(a->key == i);
		assert(a->value == 100+i);
	}
	
	// Test size
	assert(map->size == 20);
	
	// Test get with valid key
	double_hash_element *e = double_hash_get(map, 5);
	assert(e->value == 105);
	
	// Test get with invalid key
	e = double_hash_get(map, -1);
	assert(e == NULL);
	
	// Test get-and-modify
	e = double_hash_get(map, 0);
	e->value = 42;
	assert(e->value == 42);
	e = double_hash_get(map, 0);
	assert(e->value == 42);
	
	// Test overwrite
	e = double_hash_get(map, 0);
	assert(e->value == 42);
	double_hash_add(map, 0, 12);
	e = double_hash_get(map, 0);
	assert(e->value == 12);
	
	// Test update
    // ..with existing key
	e = double_hash_update(map,10,-1);
	assert(e->value == 109);
	assert(e->key == 10);
    // ..with new key
	e = double_hash_get(map,312);
	assert(e == NULL);
	e = double_hash_update(map,312,9);
	assert(e->key == 312);
	assert(e->value == 9);
	
	
	// Test iteration
	double_count = 0;
	double_hash_foreach(map, &print_double_pair);
	assert(double_count == 21);
    
    // Clean up
	double_hash_free(map);
    
    // Test dot
    double_hash *double_hash_a = double_hash_new(10);
    double_hash_add(double_hash_a, 0, 1);
    double_hash_add(double_hash_a, 1, 2);
    double_hash *double_hash_b = double_hash_new(10);
    double_hash_add(double_hash_b, 1, 1);
    double_hash_add(double_hash_b, 2, 5);
    assert(double_hash_dot(double_hash_a, double_hash_b) == 2);
    double_hash_free(double_hash_a);
    double_hash_free(double_hash_b);
    
    // Test sum
    double_hash *sum = double_hash_new(10);
    // Hash sum should start at zero
    assert(sum->sum == 0);
    // If we add a new item, sum should be its value
    double_hash_add(sum, 0, 5);
    assert(sum->sum == 5);
    // If we update that item, make sure the sum reflects the change
    double_hash_update(sum, 0, 3);
    assert(sum->sum == 8);
    // How 'bout negative updates?
    double_hash_update(sum, 0, -2);
    assert(sum->sum == 6);
    // If we add a key that already exists, make sure we subtracted the old value
    double_hash_add(sum,0,3);
    assert(sum->sum == 3);
    // Multiple values OK?
    double_hash_add(sum, 1, 1);
    assert(sum->sum == 4);
    double_hash_update(sum, 1, -1);
    assert(sum->sum == 3);
    double_hash_add(sum, 1, 5);
    assert(sum->sum == 8);
    double_hash_free(sum);
}

