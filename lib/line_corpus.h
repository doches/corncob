#ifndef LINE_CORPUS_H
#define LINE_CORPUS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "WordMap.h"
#include "statusbar.h"

#define DEFAULT_DOCUMENT_SIZE 32

#ifndef READLINE_LENGTH
#define READLINE_LENGTH 1000
#endif

#define LINE_CORPUS_WORDMAP_BUCKETS 128

typedef struct line_corpus_document_t
{
	unsigned int *words;
	unsigned int length;
	unsigned int max_length;
	struct line_corpus_document_t *next;
} line_corpus_document;

line_corpus_document *line_corpus_document_new(char *line, WordMap *wordmap);
void line_corpus_document_print(line_corpus_document *doc);

typedef struct line_corpus_t
{
	line_corpus_document *documents;
	unsigned int document_count;
	WordMap *wordmap;
} line_corpus;

line_corpus *line_corpus_new(const char *filename);
void line_corpus_each_document(line_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int));
void line_corpus_free(line_corpus *corpus);

#endif
