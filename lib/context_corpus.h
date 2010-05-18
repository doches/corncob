#ifndef CONTEXT_CORPUS_H
#define CONTEXT_CORPUS_H

#define READLINE_LENGTH 80
#define CONTEXT_BUCKETS 120

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ct_hash.h"
#include "SparseCounts.h"
#include "progressbar.h"

// A list of labeled sparsecounts -- each context_corpus_d is one row in the sparsevector file
typedef struct _context_corpus_d_t
{
	SparseCounts *counts;
	struct _context_corpus_d_t *next;
	char *label;
} context_corpus_d;

context_corpus_d *context_corpus_d_new();
void context_corpus_d_free(context_corpus_d *document);

// A list of corpus 'documents' -- each context_corpus_edoc is one column in the sparsevector file
typedef struct _context_corpus_edoc_t
{
	unsigned int *words;
	unsigned int size;
	struct _context_corpus_edoc_t *next;
} context_corpus_edoc;

context_corpus_edoc *context_corpus_edoc_new(unsigned int *words,unsigned int size);
void context_corpus_edoc_free(context_corpus_edoc *edoc);

typedef struct _context_corpus_t
{
	char *filename;
	context_corpus_d *list;
	unsigned long dims;
	unsigned long items;
	context_corpus_edoc *documents;
	unsigned long document_count;
} context_corpus;

context_corpus *context_corpus_new(char *filename);
void context_corpus_free(context_corpus *corpus);
void context_corpus_each_document(context_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int));
void context_corpus_make_documents(context_corpus *corpus);
void context_corpus_make_each_document(context_corpus *corpus);
//void SparseCount

#endif
