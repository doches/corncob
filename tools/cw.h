#ifndef CW_H
#define CW_H

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

typedef struct CW_t
{
    // algorithm to use for updates
    algorithm update_method;
    // threshold for updating category assignments
    double threshold;

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
} CW;

CW *CW_new(char *filename, algorithm alg, double threshold);
CW *CW_resume(char *focw_filename, char *corpus_filename, int document_index, algorithm alg, double threshold);
void CW_train(CW *model);
void CW_free(CW *model);
void CW_save_wordmap(CW *model);
void CW_save_categorization(CW *model);
void CW_save_meanings(CW *model);
void CW_save_target_wordmap(CW *model);
void array_shuffle(Pair *array,unsigned int size);
double_hash *CW_ppmi(CW *model,int target_index);

#endif

