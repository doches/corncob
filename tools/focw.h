/*
 *  focw.h
 *  corncob
 *
 *  Created by Trevor Fountain on 8/26/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef FOCW_H
#define FOCW_H

#include "target_corpus.h"
#include "unsigned_array.h"
#include "progressbar.h"
#include "ct_hash.h"

#define PRECISION 32

typedef struct FoCW_t
{
    target_corpus *corpus;
    ct_hash *index_to_target;
    const char *corpus_filename;
    
    unsigned_array **targets;
    unsigned int num_targets;
    
    int *categories;
    unsigned int next_category;
    
    double *unit_vectors[PRECISION];
    unsigned int k;
} FoCW;

FoCW *FoCW_new(const char *filename);
void FoCW_train_step(FoCW *model);
void FoCW_train(FoCW *model);
void FoCW_each_document(unsigned int target, unsigned int *words, unsigned int length);
unsigned_array *FoCW_get_target(FoCW *model,unsigned int index);
void FoCW_free(FoCW *model);
void FoCW_dump(FoCW *model);
void FoCW_shuffle(unsigned int *array, unsigned int n);

#endif

