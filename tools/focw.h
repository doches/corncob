/*
 *  focw.h
 *  corncob
 *
 *  Created by Trevor Fountain on 9/8/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef OCW_H
#define OCW_H

#include "target_corpus.h"
#include "unsigned_array.h"
#include "progressbar.h"
#include "ct_hash.h"
#include "ct_hash_print.h"
#include "double_hash.h"
#include "double_matrix.h"

typedef struct Pair_t
{
    unsigned int a;
    unsigned int b;
} Pair;

typedef enum {TOP, VOTE, NEAREST} algorithm;

typedef struct OCW_t
{
    // algorithm to use for updates
    algorithm update_method;
    // threshold for updating category assignments
    double threshold;
    // Output current categorization per Nth document
    unsigned int output_every_index;

    // The target corpus to use for training/testing
    target_corpus *corpus;
    char *corpus_filename;
    unsigned int document_index;
    unsigned int skip_documents;
    
    // Store target-word co-occurrence matrix in an array of arrays.
    ct_hash **targets;
    unsigned int num_targets;
    unsigned int max_targets;
    
    // For calculating PPMI
        // Count of all words seen in all contexts
    unsigned int f_xx;
        // Count of how many times each context word has appeared
    ct_hash *context_counts;
    
    // Category assigments
    unsigned_array *assignments;
    unsigned int num_categories;
    // Previous best distances
    double_matrix *distances;
    
    // Map WordMap values -> target indices
    ct_hash *wordmap_to_target;
    
    // Cache PPMI vectors
    double_hash *cached_ppmi[1000];
} OCW;

OCW *OCW_new(char *filename, algorithm alg, double threshold, int interval);
OCW *OCW_resume(char *focw_filename, char *corpus_filename, int document_index, algorithm alg, double threshold, int interval);
void OCW_train(OCW *model);
void OCW_free(OCW *model);
void OCW_save_wordmap(OCW *model);
void OCW_save_categorization(OCW *model);
void OCW_save_representations(OCW *model);
void OCW_save_meanings(OCW *model);
void OCW_save_target_wordmap(OCW *model);
void array_shuffle(Pair *array,unsigned int size);
double_hash *OCW_ppmi(OCW *model,int target_index);

#endif

