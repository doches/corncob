/*
 *  corpus.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef CORPUS
#define CORPUS 1

typedef struct _corpus_document_t
{
	char *filename;
	struct _corpus_document_t *next;
} corpus_document;

typedef struct _corpus_t
{
	corpus_document *docs;
	char *directory;
	unsigned int size;
} corpus;

corpus *corpus_new(char *directory,char *ext,unsigned int max_docs);
void corpus_free(corpus *corp);
void corpus_each_document(corpus *corp, void (*document_callback)(char *));
void document_each_word(const char *path, void (*word_callback)(char *));

#endif

