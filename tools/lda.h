/*
 *  lda.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ct_hash.h"
#include "corpus.h"
#include "word_hash.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Instance.h"

typedef struct _LDA
{
	word_hash *words;
	ct_hash **topics;
//	ct_hash *topic_counts;
  unsigned int *topic_counts;
	ct_hash **documents;
//	ct_hash *document_counts;
  unsigned int *document_counts;
	Instance *assignments;
	unsigned int n_topics;
	unsigned int n_documents;
	double alpha;
	double beta;
	
	unsigned int document_index;
} LDA;

LDA *LDA_new(int n_docs, int n_topics, int w_buckets, int t_buckets, int d_buckets, double alpha, double beta);
void LDA_delete(LDA *model);
void LDA_train_document(char *path);
word_hash_element *LDA_word_index(LDA *model,char *word);
void LDA_gibbs(LDA *model);
void LDA_dump_wordmap(LDA *model,char *prefix);
void LDA_dump_counts(LDA *model,char *prefix);
void LDA_trim_assignments(LDA *model, unsigned int threshold);
