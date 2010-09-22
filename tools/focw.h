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

typedef struct OCW_t
{
    // threshold for updating category assignments
    double threshold;
    // Output current categorization per Nth document
    unsigned int output_every_index;

    // The target corpus to use for training/testing
    target_corpus *corpus;
    char *corpus_filename;
    unsigned int document_index;
    
    // Store target-word co-occurrence matrix in an array of arrays.
    ct_hash **targets;
    unsigned int num_targets;
    unsigned int max_targets;
    
    // Category assigments
    unsigned_array *assignments;
    unsigned int num_categories;
    // Previous best distances
    double distances[550][550];
    
    // Map WordMap values -> target indices
    ct_hash *wordmap_to_target;
} OCW;

OCW *OCW_new(char *filename, double threshold, int interval);
void OCW_train(OCW *model);
void OCW_free(OCW *model);
void OCW_save_wordmap(OCW *model);
void OCW_save_categorization(OCW *model);
void OCW_save_representations(OCW *model);

#endif

