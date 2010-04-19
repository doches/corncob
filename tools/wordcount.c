#include "corpus.h"
#include "SparseCounts.h"
#include "WordMap.h"

// Function prototypes
void each_doc(char *path);
void each_word(char *word);

// Globals
WordMap *wordmap;

int main(void)
{
	corpus *toy = corpus_new("toy", "xml", 3);
	wordmap = WordMap_new(10);
	
	corpus_each_document(toy, &each_doc);
	
	corpus_free(toy);
	return 0;
}

void each_word(char *word)
{
	printf("-%s (%d)\n",word,WordMap_index(wordmap,word));
}

void each_doc(char *path)
{
	printf("%s\n",path);
	document_each_word(path, &each_word);
}
