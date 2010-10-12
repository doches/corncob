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
#include <stdio.h>

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
	hash->sum = 0;
	return hash;
}

// Internal hash function
int ct_h_hash(unsigned int key,int interval)
{
//    key = (key >> 0) * 2654435761;
	return key % interval;
}

// Add an element to the hash
hash_element *hash_add(ct_hash *map,int key,unsigned int value)
{
	unsigned int index = ct_h_hash(key,map->num_buckets);
	hash_element *chain;
    
  map->sum += value;
    
	if (map->buckets[index] == NULL) {
		map->buckets[index] = ct_h_element_new(key, value);
		map->size++;
		return map->buckets[index];
	} else {
		chain = map->buckets[index];
		while (chain->next != NULL) {
			if (chain->key == key) {
#ifdef DEBUG
				fprintf(stderr,"[warn] Replacing key %d (%d) with %d\n",key,chain->value,value);
#endif
				map->sum -= chain->value;
				chain->value = value;
				return chain;
			}
			chain = chain->next;
		}
    if (chain->key == key) {
#ifdef DEBUG
			fprintf(stderr,"[warn] Replacing key %d (%d) with %d\n",key,chain->value,value);
#endif
      map->sum -= chain->value;
      chain->value = value;
      return chain;
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
		map->sum += change;
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

void hash_print_helper(hash_element *element)
{
    printf("%d (%d), ",element->key,element->value);
}

void hash_print(ct_hash *hash)
{
    printf("ct_hash (%p) <",(void *)hash);
    hash_foreach(hash, &hash_print_helper);
    printf(">\n");
}

void hash_printx(ct_hash *hash, char *label)
{
    printf("%s <",label);
    hash_foreach(hash, &hash_print_helper);
    printf(">\n");
}

ct_hash *static_hash_intersect_other;
ct_hash *static_hash_intersection;
void hash_intersection_helper(hash_element *element)
{
  if(hash_get(static_hash_intersect_other,element->key) != NULL) {
    hash_add(static_hash_intersection,element->key,element->value);
  }
}

ct_hash *hash_intersection(ct_hash *a, ct_hash *b)
{
  ct_hash *intersect = hash_new(a->size > b->size ? a->size : b->size);
  static_hash_intersect_other = b;
  static_hash_intersection = intersect;
  hash_foreach(a,&hash_intersection_helper);
  return intersect;
}

int hash_sanity_check_bucket(hash_element *bucket)
{
	while(bucket != NULL) {
		if (bucket->next == (hash_element *)0x20) {
			printf("Failed ct_hash sanity check with 0x20 pointer\n");
			return 0;
		}
		bucket = bucket->next;
	}
	return 1;
}

int hash_sanity_check(ct_hash *a)
{
	for(int i=0;i<a->num_buckets;i++) {
		if (!hash_sanity_check_bucket(a->buckets[i])) {
			return 0;
		}	
	}
	return 1;
}
