#ifndef CONTEXT_CORPUS_H
#define CONTEXT_CORPUS_H

#define READLINE_LENGTH 80
#define CONTEXT_BUCKETS 120

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SparseCounts.h"

typedef struct _context_corpus_d_t
{
	SparseCounts *counts;
	struct _context_corpus_d_t *next;
	char *label;
} context_corpus_d;

context_corpus_d *context_corpus_d_new();
void context_corpus_d_free(context_corpus_d *document);

typedef struct _context_corpus_t
{
	char *filename;
	context_corpus_d *list;
	unsigned long dims;
	unsigned long items;
} context_corpus;

context_corpus *context_corpus_new(char *filename);
void context_corpus_free(context_corpus *corpus);
void context_corpus_each_document(context_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int));
//void SparseCount

#endif
