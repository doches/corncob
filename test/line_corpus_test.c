#include "line_corpus_test.h"

unsigned int lct_counts[3] = {3,2,44};
unsigned int lct_index = 0;
void line_corpus_each(unsigned int *words, unsigned int length)
{
	assert(lct_counts[lct_index++] == length);
}

void test_line_corpus()
{	
	line_corpus *corpus = line_corpus_new("test/data/line.corpus");
	assert(corpus != NULL);
	assert(corpus->document_count == 3);
	
	line_corpus_each_document(corpus, &line_corpus_each);
	
	line_corpus_free(corpus);
}