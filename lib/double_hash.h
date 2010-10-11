/*
 *  double_hash.h
 *  ctools
 *
 *  Created by Trevor Fountain on 6/10/10.
 *  Copyright 2010 texasexpat.net. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "WordMap.h"

#ifndef DOUBLE_HASH
#define DOUBLE_HASH

/********** Hash Element ************/

// linked-list style hashable integer
typedef struct _double_hash_element {
	int key;
	struct _double_hash_element *next;
	double value;
} double_hash_element;

/********** Hash Map ****************/

// Hash map containing N buckets (chains) of double_hash_element.
typedef struct _ct_double_hash {
	double_hash_element **buckets;
	unsigned int num_buckets;
	unsigned int size;
    unsigned int sum;
} double_hash;

double_hash *double_hash_new(unsigned int buckets);
double_hash_element *double_hash_get(double_hash *map, int key);
double_hash_element *double_hash_add(double_hash *map,int key,double value);
void double_hash_foreach(double_hash *map,void (*callback)(double_hash_element *));
void double_hash_free(double_hash *map);
double_hash_element *double_hash_update(double_hash *map, int key, double change);
double_hash_element *double_hash_reverse_lookup(double_hash *map, double value);
double double_hash_cosine(double_hash *a, double_hash *b);
void double_hash_print(double_hash *hash);
void double_hash_printx(double_hash *hash,char *label);
double_hash *double_hash_intersection(double_hash *a, double_hash *b);
void double_hash_print_labeled(double_hash *hash,char *label,WordMap *wordmap);

double double_hash_dot(double_hash *a, double_hash *b);
double double_hash_magnitude(double_hash *a);
double double_hash_cosine(double_hash *a, double_hash *b);

#endif

