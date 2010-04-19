#include "WordMap_test.h"

void test_WordMap()
{
	// Can we create successfully?
	WordMap *wordmap = WordMap_new(10);
	assert(wordmap->size == 0);
	assert(wordmap->map->num_buckets == 10);
	
	// Try some keys...
	assert(WordMap_index(wordmap, "foo") == 0);
	assert(WordMap_index(wordmap, "foo") == 0);
	assert(WordMap_index(wordmap, "bar") == 1);
	assert(WordMap_index(wordmap, "foo") == 0);
	assert(WordMap_index(wordmap, "bar") == 1);
	assert(wordmap->size == 2);
	
//	WordMap_dump(wordmap, "test.wordmap");
	
	// Clean up
	WordMap_free(wordmap);
}

