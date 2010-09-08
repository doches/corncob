/*
 *  ct_hash.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ct_hash.h"
#include <math.h>
#include <assert.h>

/*********** Hash Element **************/

// Create a new hash element
hash_element *ct_h_element_new(int key, unsigned int value) {
	hash_element *element = (hash_element *)malloc(sizeof(hash_element));
	element->key = key;
	element->value = value;
	element->next = NULL;
	return element;
}

/*********** Hash Map ******************/

// Create a new hashmap
ct_hash *hash_new(unsigned int buckets)
{
	ct_hash *hash = malloc(sizeof(ct_hash));
	hash->num_buckets = buckets;
	hash->buckets = malloc(sizeof(hash_element*)*buckets);
	for(int i=0;i<hash->num_buckets;i++) {
		hash->buckets[i] = NULL;
	}
	hash->size = 0;
	return hash;
}

// Internal hash function
int ct_h_hash(unsigned int key,int interval)
{
    key = (key >> 0) * 2654435761;
	return key % interval;
}

// Add an element to the hash
hash_element *hash_add(ct_hash *map,int key,unsigned int value)
{
	unsigned int index = ct_h_hash(key,map->num_buckets);
	hash_element *chain;
    
	if (map->buckets[index] == NULL) {
		map->buckets[index] = ct_h_element_new(key, value);
		map->size++;
		return map->buckets[index];
	} else {
		chain = map->buckets[index];
		while (chain->next != NULL) {
			if (chain->key == key) {
				chain->value = value;
				return chain;
			}
			chain = chain->next;
		}
		chain->next = ct_h_element_new(key, value);
		map->size++;
		return chain->next;
	}
}

// Update an element in the hash by adding <change>; if the element doesn't exist, it takes on the value <change>
hash_element *hash_update(ct_hash *map, int key, int change)
{
	hash_element *elem = hash_get(map,key);
	if(elem == NULL){
		elem = hash_add(map,key,change);
	} else {
		elem->value += change;
	}
	return elem;
}

// Iterate over the key-value pairs in this map
void hash_foreach(ct_hash *map,void (*callback)(hash_element *))
{
	for(int i=0;i<map->num_buckets;i++)
	{
		hash_element *item = map->buckets[i];
		while (item != NULL) {
			(*callback)(item);
			item = item->next;
		}
	}
}

// Get a value out of a hashmap
hash_element *hash_get(ct_hash *map, int key)
{
	unsigned int index = ct_h_hash(key, map->num_buckets);
	hash_element *element = map->buckets[index];
	if (element != NULL) {
		while (element != NULL && element->key != key) {
			element = element->next;
		}
	}
	return element;
}

void hash_free_helper(hash_element *element)
{
	free(element);
}

void hash_free(ct_hash *map)
{
	hash_foreach(map,&hash_free_helper);
	free(map->buckets);
	free(map);
}

// Return the first encountered hash element with the requested value.
hash_element *hash_reverse_lookup(ct_hash *map, int value)
{
	for(int i=0;i<map->num_buckets;i++)
	{
		hash_element *item = map->buckets[i];
		while (item != NULL) {
            if (item->value == value) {
                return item;
            }
			item = item->next;
		}
	}
    return NULL;
}

double hash_dot_sum;
ct_hash *hash_dot_other;
void hash_dot_helper(hash_element *elem)
{
    hash_element *other = hash_get(hash_dot_other, elem->key);
    if (other) {
        hash_dot_sum += elem->value * other->value;
    }
}
    

double hash_dot(ct_hash *a, ct_hash *b)
{
    hash_dot_sum = 0.0;
    hash_dot_other = b;
    hash_foreach(a, &hash_dot_helper);
    
    return hash_dot_sum;
}

double hash_magnitude(ct_hash *a)
{
    return sqrt(hash_dot(a,a));
}

double hash_cosine(ct_hash *a, ct_hash *b)
{
    if (a->size < b->size) {
        return hash_cosine(b,a);
    }
    return hash_dot(a,b) / (hash_magnitude(a) * hash_magnitude(b));
}