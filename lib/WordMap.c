#include "WordMap.h"

WordMap *WordMap_new(unsigned int buckets)
{
	WordMap *new = malloc(sizeof(WordMap));
	new->map = word_hash_new(buckets);
	new->size = 0;
	return new;
}

unsigned int WordMap_index(WordMap *map, char *word)
{
	word_hash_element *element = word_hash_get(map->map, word);
	if(element == NULL || element->key == NULL) // new key, add it
	{
		word_hash_add(map->map, word, map->size++);
		return map->size-1;
	} else { // existing key, return
		return element->value;
	}
}

FILE *WordMap_fout;

void WordMap_dump_helper(word_hash_element *word_e)
{
	char index[20];
	sprintf(index,"%d ",word_e->value);
	char *str = malloc(sizeof(char) * (strlen(index)+1+strlen(word_e->key)));
	strcpy(str,index);
	strcat(str,word_e->key);
	fputs(str,WordMap_fout);
	fputs("\n",WordMap_fout);
	free(str);
}

void WordMap_dump(WordMap *map, char *filename)
{
	WordMap_fout = fopen(filename,"w");
	word_hash_foreach(map->map,&WordMap_dump_helper);
	fclose(WordMap_fout);
}

void WordMap_free(WordMap *map)
{
	word_hash_free(map->map);
	free(map);
}

