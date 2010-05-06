#include "context_corpus.h"

context_corpus_d *context_corpus_d_new()
{
	context_corpus_d *new = malloc(sizeof(context_corpus_d));
	new->counts = SparseCounts_new(CONTEXT_BUCKETS);
	new->next = NULL;
	return new;
}

void context_corpus_d_free(context_corpus_d *doc)
{
	SparseCounts_free(doc->counts);
	free(doc);
}

void context_corpus_free(context_corpus *corpus)
{
	context_corpus_d *doc = corpus->list;
	context_corpus_d *prev = NULL;
	while(doc != NULL) {
		prev = doc;
		doc = doc->next;
		context_corpus_d_free(prev);
	}
	free(corpus);
}

context_corpus *context_corpus_new(char *filename)
{
	context_corpus *new = malloc(sizeof(context_corpus));
	new->filename = filename;
	new->list = NULL;
	new->items = 0;
	context_corpus_d *doc = NULL;
	
	unsigned int count = 0;
	FILE *fin = fopen(filename,"r");
	char line[READLINE_LENGTH];
	char *delim = " ";
	unsigned long max = 0;
	while(!feof(fin)) {
		fgets(line, READLINE_LENGTH, fin);
		if(strlen(line) <= 1) {
			break;
		}
		if(count <= 0) {
			char *label = strtok(line,delim);
			count = atoi((const char *)strtok(NULL,delim));
			if(doc == NULL) {
				new->list = context_corpus_d_new();
				doc = new->list;
			} else {
				doc->next = context_corpus_d_new();
				doc = doc->next;
			}
			doc->label = label;
			new->items++;
		} else {
			unsigned int index = atoi((const char *)strtok(line,delim));
			unsigned int value = atoi((const char *)strtok(NULL,delim));
			SparseCounts_add(doc->counts,index,value);
			count--;
			
			if(index > max) {
				max = index;
			}
		}
	}
	new->dims = max;
	return new;
}

void context_corpus_each_document(context_corpus *corpus, void (*document_callback)(unsigned int *, unsigned int))
{
	for(int doc=0;doc<=corpus->dims;doc++) {
		context_corpus_d *list = corpus->list;
		unsigned int docsize = 64;
		unsigned int *document = malloc(sizeof(unsigned int *)*docsize);
		unsigned int docindex = 0;
		unsigned int w_i = 0;
		while(list != NULL) {
			for(int i=0;i<SparseCounts_getValue(list->counts,doc);i++) {
				document[docindex++] = w_i;
				if(docindex >= docsize) {
					unsigned int *newdoc = malloc(sizeof(unsigned int *)*(docsize*2));
					memcpy(newdoc,document,docsize);
					docsize *= 2;
					free(document);
					document = newdoc;
				}
			}
			list = list->next;
			w_i++;
		}

		(*document_callback)(document,docindex);
		
		free(document);
	}
}

