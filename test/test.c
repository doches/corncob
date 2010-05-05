/*
 *  main.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "test.h"
#include "ct_hash_test.h"
#include "word_hash_test.h"
#include "corpus_test.h"
#include "SparseCounts_test.h"
#include "WordMap_test.h"
#include "context_corpus_test.h"

int main (int argc, const char * argv[]) {
	
	DO_TEST("corpus",test_corpus);
	DO_TEST("ct_hash",test_hash);
	DO_TEST("SparseCounts",test_SparseCounts);
	DO_TEST("word_hash",test_word_hash);
	DO_TEST("WordMap",test_WordMap);
	DO_TEST("context_corpus",test_context_corpus);
	
	printf("Pass\n");
    return 0;
}

