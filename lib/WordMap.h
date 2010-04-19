#ifndef WORD_MAP_H
#define WORD_MAP_H

#include "word_hash.h"
#include <string.h>	
#include <stdio.h>

typedef struct t_WordMap {
	word_hash *map;
	unsigned int size;
} WordMap;

WordMap *WordMap_new(unsigned int buckets);
unsigned int WordMap_index(WordMap *map, char *word);
void WordMap_free(WordMap *map);
void WordMap_dump(WordMap *map, char *filename);

#endif
