#include "SparseCounts_test.h"

void test_SparseCounts()
{
	// Allocate and initialize a new SparseCounts
	SparseCounts *counts = SparseCounts_new(10);
	// Make sure we allocated correctly...
	assert(counts != NULL);
	// ...and initialized correctly
	assert(counts->total == 0);
	assert(counts->map->size == 0);
	
	// Does getValue( <invalid key> ) return -1?
	assert(SparseCounts_getValue(counts,1) == 0);
	// Does getElement( <invalid key> ) return NULL?
	assert(SparseCounts_getElement(counts,1) == NULL);
	
	// How about valid keys?
	hash_element *added = SparseCounts_add(counts,1,2);
	// ...in getValue()?
	assert(SparseCounts_getValue(counts,1) == 2);
	// ...in getElement()?
	hash_element *valid = SparseCounts_getElement(counts,1);
	assert(valid != NULL);
	assert(valid->key == 1);
	assert(valid->value == 2);
	
	// Does add() return the correct pointer?
	assert(added == valid);
	assert(SparseCounts_add(counts,2,10) == SparseCounts_getElement(counts,2));
	
	// Can we add to an existing key?
	hash_element *existing = SparseCounts_getElement(counts,2);
	assert(existing->value == 10);
	hash_element *modified = SparseCounts_add(counts,2,5);
	assert(modified->value == 15);
	assert(modified == existing);
	assert(existing->value == 15);
	assert(SparseCounts_getValue(counts,2) == 15);
	
	// Clean up after ourselves.
	SparseCounts_free(counts);
	
	// Extended test to make sure we keep a proper total...
	counts = SparseCounts_new(5);
	int total = 0;
	assert(counts->total == 0);
	for(int i=0;i<10;i++) {
		SparseCounts_add(counts,i,i);
		total += i;
	}
	assert(counts->total == total);
	// Test subtraction
	SparseCounts_add(counts,0,-5);
	assert(counts->total == total-5);
	total += -5;
	// One more check for getValue
	assert(SparseCounts_getValue(counts,1000) == 0);
	
	SparseCounts_free(counts);
}
