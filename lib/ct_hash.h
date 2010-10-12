/*
 *  ct_hash.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 texasexpat.net. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef CT_HASH
#define CT_HASH

/********** Hash Element ************/

// linked-list style hashable integer
typedef struct _hash_element {
	int key;
	struct _hash_element *next;
	unsigned int value;
} hash_element;


/********** Hash Map ****************/

// Hash map containing N buckets (chains) of hash_element.
typedef struct _ct_h_hash {
	hash_element **buckets;
	unsigned int num_buckets;
	unsigned int size;
    unsigned int sum;
} ct_hash;

ct_hash *hash_new(unsigned int buckets);
hash_element *hash_get(ct_hash *map, int key);
hash_element *hash_add(ct_hash *map,int key,unsigned int value);
void hash_foreach(ct_hash *map,void (*callback)(hash_element *));
void hash_free(ct_hash *map);
hash_element *hash_update(ct_hash *map, int key, int change);
hash_element *hash_reverse_lookup(ct_hash *map, int value);
double hash_cosine(ct_hash *a, ct_hash *b);
void hash_print(ct_hash *hash);
void hash_printx(ct_hash *hash, char *label);

double hash_dot(ct_hash *a, ct_hash *b);
double hash_magnitude(ct_hash *a);
double hash_cosine(ct_hash *a, ct_hash *b);
ct_hash *hash_intersection(ct_hash *a, ct_hash *b);

int hash_sanity_check(ct_hash *a);

#endif

