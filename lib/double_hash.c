/*
 *  double_hash.c
 *  corncob
 *
 *  Created by Trevor Fountain on 10/6/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "double_hash.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

/*********** Hash Element **************/

// Create a new hash element
double_hash_element *double_hash_element_new(int key, double value) {
	double_hash_element *element = (double_hash_element *)malloc(sizeof(double_hash_element));
	element->key = key;
	element->value = value;
	element->next = NULL;
	return element;
}

/*********** Hash Map ******************/

// Create a new hashmap
double_hash *double_hash_new(unsigned int buckets)
{
	double_hash *hash = malloc(sizeof(double_hash));
	hash->num_buckets = buckets;
	hash->buckets = malloc(sizeof(double_hash_element*)*buckets);
	for(int i=0;i<hash->num_buckets;i++) {
		hash->buckets[i] = NULL;
	}
	hash->size = 0;
    hash->sum = 0;
	return hash;
}

// Internal hash function
int double_h_hash(unsigned int key,int interval)
{
    //    key = (key >> 0) * 2654435761;
	return key % interval;
}

// Add an element to the hash
double_hash_element *double_hash_add(double_hash *map,int key,double value)
{
	unsigned int index = double_h_hash(key,map->num_buckets);
	double_hash_element *chain;
    
    map->sum += value;
    
	if (map->buckets[index] == NULL) {
		map->buckets[index] = double_hash_element_new(key, value);
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
		chain->next = double_hash_element_new(key, value);
		map->size++;
		return chain->next;
	}
}

// Update an element in the hash by adding <change>; if the element doesn't exist, it takes on the value <change>
double_hash_element *double_hash_update(double_hash *map, int key, double change)
{
	double_hash_element *elem = double_hash_get(map,key);
	if(elem == NULL){
		elem = double_hash_add(map,key,change);
	} else {
		elem->value += change;
        map->sum += change;
	}
	return elem;
}

// Iterate over the key-value pairs in this map
void double_hash_foreach(double_hash *map,void (*callback)(double_hash_element *))
{
	for(int i=0;i<map->num_buckets;i++)
	{
		double_hash_element *item = map->buckets[i];
		while (item != NULL) {
			(*callback)(item);
			item = item->next;
		}
	}
}

// Get a value out of a hashmap
double_hash_element *double_hash_get(double_hash *map, int key)
{
	unsigned int index = double_h_hash(key, map->num_buckets);
	double_hash_element *element = map->buckets[index];
	if (element != NULL) {
		while (element != NULL && element->key != key) {
			element = element->next;
		}
	}
	return element;
}

void double_hash_free_helper(double_hash_element *element)
{
	free(element);
}

void double_hash_free(double_hash *map)
{
	double_hash_foreach(map,&double_hash_free_helper);
	free(map->buckets);
	free(map);
}

// Return the first encountered hash element with the requested value.
double_hash_element *double_hash_reverse_lookup(double_hash *map, double value)
{
	for(int i=0;i<map->num_buckets;i++)
	{
		double_hash_element *item = map->buckets[i];
		while (item != NULL) {
            if (item->value == value) {
                return item;
            }
			item = item->next;
		}
	}
    return NULL;
}

double double_hash_dot_sum;
double_hash *double_hash_dot_other;
void double_hash_dot_helper(double_hash_element *elem)
{
    double_hash_element *other = double_hash_get(double_hash_dot_other, elem->key);
    if (other) {
        double_hash_dot_sum += elem->value * other->value;
    }
}

double double_hash_dot(double_hash *a, double_hash *b)
{
    double_hash_dot_sum = 0.0;
    double_hash_dot_other = b;
    double_hash_foreach(a, &double_hash_dot_helper);
    
    return double_hash_dot_sum;
}

double double_hash_magnitude(double_hash *a)
{
    return sqrt(double_hash_dot(a,a));
}

double double_hash_cosine(double_hash *a, double_hash *b)
{
    if (a->size < b->size) {
        return double_hash_cosine(b,a);
    }
    return double_hash_dot(a,b) / (double_hash_magnitude(a) * double_hash_magnitude(b));
}

void double_hash_print_helper(double_hash_element *element)
{
    printf("%d (%f), ",element->key,element->value);
}

void double_hash_print(double_hash *hash)
{
    printf("double_hash (%p) <",(void *)hash);
    double_hash_foreach(hash, &double_hash_print_helper);
    printf(">\n");
}

void double_hash_printx(double_hash *hash, char *label)
{
    printf("%s <",label);
    double_hash_foreach(hash, &double_hash_print_helper);
    printf(">\n");
}

double_hash *static_double_hash_intersect_other;
double_hash *static_double_hash_intersection;
void double_hash_intersection_helper(double_hash_element *element)
{
    if(double_hash_get(static_double_hash_intersect_other,element->key) != NULL) {
        double_hash_add(static_double_hash_intersection,element->key,element->value);
    }
}

double_hash *double_hash_intersection(double_hash *a, double_hash *b)
{
    double_hash *intersect = double_hash_new(a->size > b->size ? a->size : b->size);
    static_double_hash_intersect_other = b;
    static_double_hash_intersection = intersect;
    double_hash_foreach(a,&double_hash_intersection_helper);
    return intersect;
}

WordMap *static_double_hash_print_wordmap;
void double_hash_print_labeled_helper(double_hash_element *element)
{
    printf("%s (%f), ",WordMap_reverse_lookup(static_double_hash_print_wordmap, element->key),element->value);
}

void double_hash_print_labeled(double_hash *hash,char *label,WordMap *wordmap)
{
    printf("%s <",label);
    
    static_double_hash_print_wordmap = wordmap;
    double_hash_foreach(hash, &double_hash_print_labeled_helper);
    static_double_hash_print_wordmap = NULL;
    printf(">\n");
}