/*
 *  ct_hash.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ct_hash.h"

/*********** Hash Element **************/

// Create a new hash element
hash_element *ct_h_element_new(int key, unsigned int value) {
	hash_element *element = malloc(sizeof(hash_element));
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
	for(int i=0;i<buckets;i++) {
		hash->buckets[i] = NULL;
	}
	hash->size = 0;
	return hash;
}

// Internal hash function
int ct_h_hash(unsigned int key,int interval)
{
	key = (key >> 0) * 2654435761;
//	return key % interval;
	return key % interval;
}

// Add an element to the hash
hash_element *hash_add(ct_hash *map,int key,unsigned int value)
{
	unsigned int index = ct_h_hash(key,map->num_buckets);
	
	if (map->buckets[index] == NULL) {
		map->buckets[index] = ct_h_element_new(key, value);
		map->size++;
#ifdef DEBUG
		printf("Adding [%d:%d] to new chain %d\n",key,value,index);
#endif
	} else {
#ifdef DEBUG
		int length = 0;
#endif
		hash_element *chain = map->buckets[index];
		while (chain->next != NULL) {
			if (chain->key == key) {
				chain->value = value;
#ifdef DEBUG
				printf("Overwriting existing key %d in chain %d\n",key,index);
#endif
				return chain;
			}
			chain = chain->next;
#ifdef DEBUG
			length++;
#endif
		}
		if (chain->key == key) {
			chain->value = value;
#ifdef DEBUG
			printf("Overwriting existing key %d in chain %d\n",key,index);
#endif
			return chain;
		}
		chain->next = ct_h_element_new(key, value);
		map->size++;
#ifdef DEBUG
		printf("Adding [%d:%d] to chain %d (length %d)\n",key,value,index,length);
#endif
	}
	return map->buckets[index];
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
//	printf("[hash]: %d hashes to %d\n",key,index);
	hash_element *element = map->buckets[index];
	if (element) {
		while (element && element->key != key) {
			element = element->next;
		}
	}
	hash_element zero;
	zero.key = -1	;
	zero.value = 0;
	zero.next = NULL;
	if (!element) {
		element = &zero;
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
