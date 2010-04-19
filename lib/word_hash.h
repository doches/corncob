/*
 *  word_hash.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 texasexpat.net. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

//#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef WORD_HASH
#define WORD_HASH

/********** Hash Element ************/

// linked-list style hashable integer
typedef struct _word_hash_element {
	char *key;
	struct _word_hash_element *next;
	unsigned int value;
} word_hash_element;


/********** Hash Map ****************/

// Hash map containing N buckets (chains) of hash_element.
typedef struct _word_hash {
	word_hash_element **buckets;
	unsigned int num_buckets;
	unsigned int size;
} word_hash;

word_hash *word_hash_new(unsigned int buckets);
word_hash_element *word_hash_get(word_hash *map, const char *key);
word_hash_element *word_hash_add(word_hash *map,char *key,unsigned int value);
void word_hash_foreach(word_hash *map,void (*callback)(word_hash_element *));
void word_hash_free(word_hash *map);

#endif

