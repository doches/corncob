#include "context_corpus.h"
#include <assert.h>

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

context_corpus_edoc *context_corpus_edoc_new(unsigned int *words, unsigned int size)
{
	context_corpus_edoc *new = malloc(sizeof(context_corpus_edoc));
	new->words = words;
	new->size = size;
	new->next = NULL;
	return new;
}

void context_corpus_edoc_free(context_corpus_edoc *edoc)
{
	free(edoc->words);
	free(edoc);
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
	if(corpus->documents != NULL) {
		context_corpus_edoc *edoc;
		while(corpus->documents != NULL) {
			edoc = corpus->documents->next;
			context_corpus_edoc_free(corpus->documents);
			corpus->documents = edoc;
		}
	}
	free(corpus);
}

context_corpus *context_corpus_new(char *filename)
{
	context_corpus *new = malloc(sizeof(context_corpus));
	new->filename = filename;
	new->list = NULL;
	new->items = 0;
	new->documents = NULL;
	new->document_count = 0;
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
			char *ct = strtok(NULL,delim);
			if(label[0] < 'a' || label[0] > 'z') {
				break;
			}
			count = atoi((const char *)ct);
			if(doc == NULL) {
				new->list = context_corpus_d_new();
				doc = new->list;
			} else {
				doc->next = context_corpus_d_new();
				doc = doc->next;
			}
			assert(label[0] >= 'a' && label[0] <= 'z');
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

void context_corpus_make_documents(context_corpus *corpus)
{
	if(corpus->documents != NULL) {
		return; // Don't re-create the documents if we've got a cache already!
	}
	
	context_corpus_make_each_document(corpus);
}

unsigned int *gcf;
void cc_lcd_fe(hash_element *elem)
{
	printf("[%d: %d]\n",elem->key, elem->value);
}

unsigned int context_corpus_lcd_document(unsigned int *document, unsigned int size)
{
	ct_hash *unique = hash_new(128);
	for(int i=0;i<size;i++) {
		hash_update(unique,document[i],1);
	}
	gcf = malloc(sizeof(unsigned int)*10);
//	hash_foreach(unique,&cc_lcd_fe);
//	printf("------------\n");
	return size;
}

void context_corpus_make_each_document(context_corpus *corpus)
{
	context_corpus_edoc *last = corpus->documents;
	progressbar *progress = progressbar_new("Loading",corpus->dims);
	for(int doc=0;doc<=corpus->dims;doc++) {
		context_corpus_d *list = corpus->list;
		unsigned int docsize = 32;
		unsigned int *document = malloc(sizeof(unsigned int)*docsize);
		unsigned int docindex = 0;
		unsigned int w_i = 0;
		
		while(list != NULL) {
			for(int i=0;i<SparseCounts_getValue(list->counts,doc);i++) {
				document[docindex] = w_i;
				docindex++;
				if(docindex >= docsize) {
					unsigned int *newdoc = malloc(sizeof(unsigned int)*(docsize*2));
					//memcpy(newdoc,document,docsize);
					for(int k=0;k<docindex;k++) {
						newdoc[k] = document[k];
					}
					docsize *= 2;
					free(document);
					document = newdoc;
				}
			}
			list = list->next;
			w_i++;
		}
		
		// Sanity check
		for(int q=0;q<docindex;q++) {
			assert(document[q] <= w_i);
		}
		
		if(docindex > 1) {
//			docindex = context_corpus_lcd_document(document,docindex);
			context_corpus_edoc *edoc = context_corpus_edoc_new(document,docindex);
			if(corpus->documents == NULL) {
				corpus->documents = edoc;
				last = edoc;
			} else {
				last->next = edoc;
				last = edoc;
			}
			corpus->document_count++;
		}
		progressbar_inc(progress);
	}
	progressbar_finish(progress);
}

void context_corpus_each_document(context_corpus *corpus, void (*document_callback)(unsigned int *words, unsigned int size))
{
	context_corpus_make_documents(corpus);
	context_corpus_edoc *edoc = corpus->documents;
	while(edoc != NULL) {
		(*document_callback)(edoc->words,edoc->size);
		edoc = edoc->next;
	}
}
