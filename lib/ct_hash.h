/*
 *  ct_hash.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 texasexpat.net. All rights reserved.
 *
 */

#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

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
} ct_hash;

ct_hash *hash_new(unsigned int buckets);
hash_element *hash_get(ct_hash *map, int key);
hash_element *hash_add(ct_hash *map,int key,unsigned int value);
void hash_foreach(ct_hash *map,void (*callback)(hash_element *));
void hash_free(ct_hash *map);
hash_element *hash_update(ct_hash *map, int key, int change);

#endif

