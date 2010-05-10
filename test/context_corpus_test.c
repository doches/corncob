/*
 *  corpus_test.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "context_corpus_test.h"

unsigned int cctotals[4] = {4,3,5,11};
unsigned int cctotals_i = 0;
void cc_each_doc(unsigned int *document, unsigned int docsize)
{
	if(cctotals_i == 0) {
		assert(document[0] == 0);
		assert(document[1] == 1);
		assert(document[2] == 1);
	}
	assert(cctotals[cctotals_i++] == docsize);
}

void test_context_corpus()
{
	context_corpus *corp = context_corpus_new("test/test.sparseline");
	
	assert(corp->dims == 3);
	assert(corp->items == 3);
	assert(corp->list != NULL);
	assert(corp->list->next != NULL);
	assert(corp->list->next->next != NULL);
	assert(corp->list->next->next->next == NULL);
	
	context_corpus_each_document(corp, &cc_each_doc);
	
	context_corpus_free(corp);
}

