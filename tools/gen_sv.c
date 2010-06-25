/*
 *  gensv.c
 *  corncob
 *
 *  Created by Trevor Fountain on 6/25/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "line_corpus.h"
#include "count_list.h"
#include "SparseCounts.h"
#include "progressbar.h"

#define SPARSE_COUNTS_BUCKETS 128

count_list *list;
progressbar *progress = NULL;
FILE *fout = NULL;

void each_document(unsigned int *words, unsigned int size);
void dump(word_hash_element *element);

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: gen_sv <line corpus> <output>\n");
		printf("  (%d arguments received)\n",argc);
		exit(1);
	}
	line_corpus *corpus = line_corpus_new(argv[1]);
	
	list = count_list_new();
	progress = progressbar_new("Counting", corpus->document_count);
	line_corpus_each_document(corpus, &each_document);
	progressbar_finish(progress);
	
	fout = fopen(argv[2],"w");
	progress = progressbar_new("Saving",corpus->wordmap->size);
	word_hash_foreach(corpus->wordmap->map, &dump); 
	progressbar_finish(progress);
	fclose(fout);
	
	line_corpus_free(corpus);
}

void dump_vector(hash_element *e)
{
	fprintf(fout,"%d %d ",e->key,e->value);
}

void dump(word_hash_element *element)
{
	fprintf(fout,"%s ",element->key);
	SparseCounts *counts = count_list_get(list, element->value);
	hash_foreach(counts->map, &dump_vector);
	fprintf(fout,"\n");
	progressbar_inc(progress);
}

void each_document(unsigned int *words, unsigned int size)
{
	for (int w_i=0; w_i<size; w_i++) {
		SparseCounts *counts = count_list_get(list, words[w_i]);
		if (counts == NULL) {
			counts = SparseCounts_new(SPARSE_COUNTS_BUCKETS);
			count_list_set(list, counts, words[w_i]);
		}
		for (int o_i=0; o_i<size; o_i++) {
			if (w_i != o_i) {
				SparseCounts_add(counts, words[o_i], 1);
			}
		}
	}
	progressbar_inc(progress);
}