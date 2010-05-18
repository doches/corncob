/*
 *  word_hash.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "word_hash.h"

/*********** Hash Element **************/

// Create a new hash element
word_hash_element *word_hash_element_new(char *key, unsigned int value) {
	word_hash_element *element = malloc(sizeof(word_hash_element));
	element->key = malloc(sizeof(char)*(strlen(key)+1));
	strcpy(element->key,key);
	element->value = value;
	element->next = NULL;
	return element;
}



/*********** Hash Map ******************/

// Create a new hashmap
word_hash *word_hash_new(unsigned int buckets)
{
	word_hash *hash = malloc(sizeof(word_hash));
	hash->num_buckets = buckets;
	hash->buckets = malloc(sizeof(word_hash_element*)*buckets);
	hash->size = 0;
	for(int i=0;i<buckets;i++) {
		hash->buckets[i] = NULL;
	}
	return hash;
}

// Internal hash function
int word_hash_hash(char *key,int interval)
{
	unsigned int hash = 0;
	int c;
	
#ifdef DEBUG
	char *orig = key;
#endif
	
	while ((c = (*(key++)))) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	
#ifdef DEBUG
	printf("Hashing %s -> %d\n",orig,hash%interval);
#endif
	
	return hash % interval;
}

// Add an element to the hash
word_hash_element *word_hash_add(word_hash *map,char *key,unsigned int value)
{
	unsigned int index = word_hash_hash(key,map->num_buckets);
	
	if (map->buckets[index] == NULL) {
		map->buckets[index] = word_hash_element_new(key, value);
		map->size++;
		
		#ifdef DEBUG
		printf("Adding [%s:%d] to new chain %d\n",key,value,index);
		#endif
		return map->buckets[index];
	} else {
		word_hash_element *chain = map->buckets[index];
		while (chain->next != NULL) {
			if (!strcmp(chain->key, key)) {
				chain->value = value;
				return chain;
			}
			chain = chain->next;
		}
		if (!strcmp(chain->key, key)) {
			chain->value = value;
			
			return chain;
		}
//		chain->next = word_hash_element_new(key, value);
		word_hash_element *new = word_hash_element_new(key, value);
		new->next = map->buckets[index];
		map->buckets[index] = new;
		
		map->size++;
		
		#ifdef DEBUG
		printf("Adding [%s:%d] to chain %d\n",key,value,index);
		#endif
		return map->buckets[index];
	}
}

// Iterate over the key-value pairs in this map
void word_hash_foreach(word_hash *map,void (*callback)(word_hash_element *))
{
	for(int i=0;i<map->num_buckets;i++)
	{
		word_hash_element *item = map->buckets[i];
		while (item != NULL) {
			(*callback)(item);
			item = item->next;
		}
	}
}

// Get a value out of a hashmap
word_hash_element *word_hash_get(word_hash *map, const char *key)
{
	unsigned int index = word_hash_hash((char *)key, map->num_buckets);
	word_hash_element *element = map->buckets[index];
	word_hash_element *prev = NULL;
	if (element) {
		while (element && strcmp(element->key,key)) {
			prev = element;
			element = element->next;
		}
	}
	if (!element) {
		return NULL;
	} else if(prev != NULL) {
		// If we actually found something, move it to the head of its bucket
		prev->next = element->next;
		element->next = map->buckets[index];
		map->buckets[index] = element;
	}
	return element;
}

void word_hash_free_helper(word_hash_element *element)
{
	free(element);
}

void word_hash_free(word_hash *map)
{
	word_hash_foreach(map, &word_hash_free_helper);
	free(map);
}
