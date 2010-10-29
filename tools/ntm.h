/*
 *  ntm.h
 *  corncob
 *
 *  Created by Trevor Fountain on 10/29/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "line_corpus.h"
#include "count_list.h"
#include "ct_hash.h"
#include "unsigned_array.h"

typedef unsigned int TopicIndex;
typedef unsigned int DocumentIndex;
typedef unsigned int WordIndex;

typedef struct nTM_t {
    double alpha;
    double beta;
    double gamma;
    
    int interval;
    
    const char *filename;
    line_corpus *corpus;
    count_list *nzws;
    count_list *ndzs;
    SparseCounts *nzs;
    
    ct_hash *unique_words;
    unsigned_array *assignments;
    
    TopicIndex topics;
    DocumentIndex documents;
    WordIndex words;
} nTM;

nTM *nTM_new(const char *filename, double alpha, double beta, double gamma);
void nTM_free(nTM *model);
void nTM_train(nTM *model);
void nTM_save_wordmap(nTM *model);
void nTM_save_assignments(nTM *model);

TopicIndex nTM_sample_topic(nTM *model, WordIndex word, DocumentIndex document);
void nTM_see_word(nTM *model, WordIndex word);
void nTM_create_topic(nTM *model);
void nTM_assign_topic(nTM *model, WordIndex word, TopicIndex topic, DocumentIndex document);

double nTM_P_d(nTM *model, DocumentIndex document);
double nTM_P_z(nTM *model, TopicIndex topic);
double nTM_P_w(nTM *model, WordIndex word);
double nTM_P_z_d(nTM *model, TopicIndex topic, DocumentIndex document);
double nTM_P_w_z(nTM *model, WordIndex word, TopicIndex topic);
double nTM_P_z_w(nTM *model, TopicIndex topic, WordIndex word);
double nTM_P_znew_d(nTM *model, DocumentIndex document);