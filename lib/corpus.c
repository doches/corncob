/*
 *  corpus.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "corpus.h"

corpus_document *corpus_document_new(char *filename)
{
	corpus_document *doc = malloc(sizeof(corpus_document));
	doc->filename = malloc(sizeof(char)*(strlen(filename)+1));
	strcpy(doc->filename,filename);
	return doc;
}

corpus *corpus_new(char *directory,char *ext,unsigned int max_docs)
{
	corpus *corp = malloc(sizeof(corpus));
	corp->directory = directory;
	corp->size = 0;
	corpus_document *prev = NULL;
	corpus_document *first = NULL;
	DIR *d;
	struct dirent *dir;
	d = opendir(directory);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strstr(dir->d_name, ext) != NULL) {
				corpus_document *doc = corpus_document_new(dir->d_name);
				if (prev) {
					prev->next = doc;
				}
				prev = doc;
				if (first == NULL) {
					first = doc;
				}
				corp->size++;
				if(max_docs >0 && corp->size >= max_docs) {
				  break;
				}
			}
		}
		corp->docs = first;
		closedir(d);
	}
	
	return corp;
}

void corpus_each_document(corpus *corp, void (*document_callback)(char *))
{
	corpus_document *doc = corp->docs;
	while (doc != NULL) {
		char *path = malloc(sizeof(char)*(strlen(corp->directory)+strlen((const char *)doc->filename)+2));
		path = strcpy(path,(const char *)corp->directory);
		path = strcat(path,"/");
		path = strcat(path,(const char *)doc->filename);
		(*document_callback)(path);
		free(path);
		doc = doc->next;
	}
}

void document_each_word(const char *path, void (*word_callback)(char *))
{
	FILE *doc = fopen(path, "r");
	char line[80];

	while (!feof(doc)) {
		fgets(line, 80, doc);
		int length = strlen(line);
		if (line[length-1] == '\n') {
			line[length-1] = '\0';
		}
		if (feof(doc)) {
			break;
		}
		(*word_callback)(line);
	}
	fclose(doc);
}

void corpus_free(corpus *corp)
{
	corpus_document *doc = corp->docs;
	while (doc != NULL) {
		corpus_document *next = doc->next;
		free(doc);
		doc = next;
	}
}

