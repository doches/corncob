/*
 *  target_corpus.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "target_corpus.h"

/***** target_corpus_document *****/

target_corpus_document *target_corpus_document_new(char *line, WordMap *wordmap)
{
	target_corpus_document *new = (target_corpus_document *)malloc(sizeof(target_corpus_document));
	new->next = NULL;
	new->words = (unsigned int *)malloc(sizeof(unsigned int)*DEFAULT_DOCUMENT_SIZE);
	new->length = 0;
	new->max_length = DEFAULT_DOCUMENT_SIZE;
    
    // Scan the line, splitting out the target word and the context words
	char *delim = " \n";
	char *word = strtok(line,delim);
    new->target = WordMap_index(wordmap,word);
    word = strtok(NULL,delim);
	do {
		if (word == NULL) {
			break;
		}
		unsigned int index = WordMap_index(wordmap,word);
		word = strtok(NULL,delim);
		new->words[new->length++] = index;
		
		// Dynamically resize words array
		if(new->length >= new->max_length) {
			new->max_length *= 2;
			unsigned int *resize = (unsigned int *)malloc(sizeof(unsigned int)*new->max_length);
			for(int i=0;i<new->length;i++) {
				resize[i] = new->words[i];
			}
			free(new->words);
			new->words = resize;
		}
	} while(word != NULL);
	return new;
}

void target_corpus_document_print(target_corpus_document *doc)
{
	printf("%p(%d):[%d",doc,doc->target,doc->words[0]);
	for(int i=1;i<doc->length;i++) {
		printf(", %d",doc->words[i]);
	}
	printf("] (%d/%d)\n",doc->length,doc->max_length);
}

void target_corpus_document_free(target_corpus_document *doc)
{
	if(doc != NULL) {
		target_corpus_document_free(doc->next);
		free(doc->words);
		free(doc);
	}
}

/***** target_corpus *****/

target_corpus *target_corpus_new(char *filename)
{
	target_corpus *new = (target_corpus *)malloc(sizeof(target_corpus));
	new->documents = NULL;
	new->document_count = 0;
	new->wordmap = WordMap_new(TARGET_CORPUS_WORDMAP_BUCKETS);
	target_corpus_document *last = NULL;
    
	FILE *fin = fopen(filename,"r");
	char line[READLINE_LENGTH];
    
    statusbar *progress = statusbar_new("Reading corpus");
	while(!feof(fin)) {
		fgets(line, READLINE_LENGTH, fin);
		
        if (strlen(line) > 1) {
            target_corpus_document *doc = target_corpus_document_new(line,new->wordmap);
            if(new->documents == NULL) {
                new->documents = doc;
            } else {
                last->next = doc;
            }
            last = doc;
            
            new->document_count++;
            if (new->document_count > 0 && new->document_count % 1000 == 0) {
                statusbar_inc(progress);
            }
        }
	}
    statusbar_finish(progress);
	fclose(fin);
	
	return new;
}

void target_corpus_each_document(target_corpus *corpus, void (*document_callback)(unsigned int,unsigned int *,unsigned int))
{
	target_corpus_document *doc = corpus->documents;
	while(doc != NULL) {
		(*document_callback)(doc->target,doc->words,doc->length);
		doc = doc->next;
	}
}

void target_corpus_free(target_corpus *corpus)
{
	target_corpus_document_free(corpus->documents);
	WordMap_free(corpus->wordmap);
	free(corpus);
}