/*
 *  target_corpus.h
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef TARGET_CORPUS_H
#define TARGET_CORPUS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "WordMap.h"
#include "statusbar.h"

#define DEFAULT_DOCUMENT_SIZE 32
#define READLINE_LENGTH 1000
#define TARGET_CORPUS_WORDMAP_BUCKETS 256

typedef struct target_corpus_document_t
{
	unsigned int *words;
	unsigned int length;
	unsigned int max_length;
    unsigned int target;
	struct target_corpus_document_t *next;
} target_corpus_document;

target_corpus_document *target_corpus_document_new(char *line, WordMap *wordmap);
void target_corpus_document_print(target_corpus_document *doc);

typedef struct target_corpus_t
{
	target_corpus_document *documents;
	unsigned int document_count;
	WordMap *wordmap;
} target_corpus;

target_corpus *target_corpus_new(char *filename);
void target_corpus_each_document(target_corpus *corpus, void (*document_callback)(unsigned int,unsigned int *,unsigned int));
void target_corpus_free(target_corpus *corpus);

#endif
