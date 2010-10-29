#include "line_corpus.h"

/***** line_corpus_document *****/

line_corpus_document *line_corpus_document_new(char *line, WordMap *wordmap)
{
	line_corpus_document *new = (line_corpus_document *)malloc(sizeof(line_corpus_document));
	new->next = NULL;
	new->words = (unsigned int *)malloc(sizeof(unsigned int)*DEFAULT_DOCUMENT_SIZE);
	new->length = 0;
	new->max_length = DEFAULT_DOCUMENT_SIZE;

	char *delim = " \n";
	char *word = strtok(line,delim);
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

void line_corpus_document_print(line_corpus_document *doc)
{
	printf("%p:[%d",doc,doc->words[0]);
	for(int i=1;i<doc->length;i++) {
		printf(", %d",doc->words[i]);
	}
	printf("] (%d/%d)\n",doc->length,doc->max_length);
}

void line_corpus_document_free(line_corpus_document *doc)
{
	if(doc != NULL) {
		line_corpus_document_free(doc->next);
//		free(doc->words);
		free(doc);
	}
}

/***** line_corpus *****/

line_corpus *line_corpus_new(const char *filename)
{
	line_corpus *new = (line_corpus *)malloc(sizeof(line_corpus));
	new->documents = NULL;
	new->document_count = 0;
	new->wordmap = WordMap_new(LINE_CORPUS_WORDMAP_BUCKETS);
	line_corpus_document *last = NULL;

	FILE *fin = fopen(filename,"r");
	char line[READLINE_LENGTH];

    statusbar *progress = statusbar_new("Reading corpus");
	while(!feof(fin)) {
		char *ret = fgets(line, READLINE_LENGTH, fin);
		if (ret == NULL) {
			break;
		}
		
		line_corpus_document *doc = line_corpus_document_new(line,new->wordmap);
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
    statusbar_finish(progress);
	fclose(fin);
	
	return new;
}

void line_corpus_each_document(line_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int))
{
	line_corpus_document *doc = corpus->documents;
	while(doc != NULL) {
		(*document_callback)(doc->words,doc->length);
		doc = doc->next;
	}
}

void line_corpus_free(line_corpus *corpus)
{
	line_corpus_document_free(corpus->documents);
	WordMap_free(corpus->wordmap);
	free(corpus);
}

