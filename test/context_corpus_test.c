/*
 *  corpus_test.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "context_corpus_test.h"

void test_context_corpus()
{
	context_corpus *corp = context_corpus_new("test/test.sparseline");
	
	assert(corp->dims == 3);
	assert(corp->items == 2);
	assert(corp->list != NULL);
	assert(corp->list->next != NULL);
	assert(corp->list->next->next == NULL);
	
	context_corpus_free(corp);
}

