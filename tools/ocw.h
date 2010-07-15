/*
 *  ocw.h
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef OCW_H
#define OCW_H

#include "target_corpus.h"
#include "unsigned_array.h"
#include "progressbar.h"
#include "ct_hash.h"
#include "double_matrix.h"

typedef struct OCW_t
{
    target_corpus *corpus;
    ct_hash *index_to_target;
    
    // Store target-word co-occurrence matrix in an array of arrays.
    unsigned_array **targets;
    unsigned int num_targets;
    unsigned int max_targets;
    
    // Category assigments
    unsigned_array *assignments;
    unsigned int num_categories;
    
    // Distances
    double_matrix *distances;
} OCW;

OCW *OCW_new(char *filename);
void OCW_train_step(OCW *model);
void OCW_train(OCW *model);
void OCW_free(OCW *model);
void OCW_dump(OCW *model);

#endif

