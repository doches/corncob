/*
 *  document_corpus.h
 *  corncob
 *
 *  Created by Trevor Fountain on 11/10/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef DOCUMENT_CORPUS_H
#define DOCUMENT_CORPUS_H

#define READLINE_LENGTH 800

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ct_hash.h"
#include "SparseCounts.h"
#include "progressbar.h"
#include "WordMap.h"

typedef struct _document_corpus_document_t
{
    char *filename;
    struct _document_corpus_document_t *next;
} document_corpus_document;

document_corpus_document *document_corpus_document_new(char *directory, char *filename);

typedef struct _document_corpus_t
{
    char *filename;
    document_corpus_document *documents;
    unsigned int document_count;
    WordMap *wordmap;
} document_corpus;

document_corpus *document_corpus_new(char *directory);
void document_corpus_each_document(document_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int));

#endif