/*
 *  word_hash_test.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "word_hash_test.h"

int wcount = 0;
void wprint_pair(word_hash_element *element)
{
	//printf("%d:\t%d\n",element->key,element->value);
	wcount++;
}

void test_word_hash()
{
	// Create a hash and populate it
	word_hash *map = word_hash_new(3);
	assert(map->size == 0);
	word_hash_add(map, "spam", 1);
	word_hash_add(map, "foo", 2);
	word_hash_add(map, "bar", 3);
	word_hash_add(map, "baz", 4);
	word_hash_add(map, "fob", 5);
	word_hash_add(map, "far", 6);
	word_hash_add(map, "foz", 7);
	word_hash_add(map, "zor", 8);

	// Test size
	assert(map->size == 8);
	
	// Test get with invalid key
	word_hash_element *e = word_hash_get(map, "eggs");
	assert(e==NULL);
	
	// Test get with valid key
	e = word_hash_get(map, "foo");
	assert(e->value == 2);	
	
	// Test get-and-modify
	e = word_hash_get(map, "bar");
	e->value = 42;
	assert(e->value == 42);
	e = word_hash_get(map, "bar");
	assert(e->value == 42);
	
	// Test overwrite
	e = word_hash_get(map, "bar");
	assert(e->value == 42);
	word_hash_add(map, "bar", 12);
	e = word_hash_get(map, "bar");
	assert(e->value == 12);
	
	// Test iteration
	wcount = 0;
	word_hash_foreach(map, &wprint_pair);
	assert(wcount == 8);
	
	word_hash_free(map);
}

