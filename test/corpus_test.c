/*
 *  corpus_test.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "corpus_test.h"

int corpus_count = 0;
void print_word(char *word)
{
	corpus_count++;
}

void each_doc(char *doc)
{
	document_each_word(doc, &print_word);
}

void test_corpus()
{
	corpus *corp = corpus_new("test/data","txt",10);
	corpus_count = 0;
	corpus_each_document(corp, &each_doc);
	assert(corpus_count == 10);
	corpus_free(corp);
}

