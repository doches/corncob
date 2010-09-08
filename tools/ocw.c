/*
 *  ocw.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "ocw.h"
#include "cosine.h"

int main(int argc, char **argv)
{
	if(argc != 2) {
		printf("ocw expects 1 argument, recieved %d\n",argc-1);
		printf("\nUsage: ocw <corpus>\n");
		return 1;
	}
    
    srand(time(0));
    OCW *model = OCW_new(argv[1]);
    OCW_train(model);
    OCW_dump(model);
    OCW_free(model);

	return 0;
}


OCW *OCW_new(char *filename)
{
    OCW *model = (OCW *)malloc(sizeof(OCW));
    model->corpus = target_corpus_new(filename);
    model->corpus_filename = filename;
    model->num_targets = 0;
    model->max_targets = 512;
    model->targets = (unsigned_array **)malloc(sizeof(unsigned_array *)*model->max_targets);
    model->index_to_target = hash_new(2);
    
    model->num_categories = 0;
    model->assignments = unsigned_array_new(512);
    
    model->distances = double_matrix_new(512, 512, 0.0);
    
    return model;
}

void OCW_free(OCW *model)
{
    target_corpus_free(model->corpus);
    hash_free(model->index_to_target);
    for (int i=0; i<model->num_targets; i++) {
        unsigned_array_free(model->targets[i]);
    }
    free(model->targets);
    free(model);
}

unsigned_array *OCW_get_target(OCW *model, unsigned int word_index)
{
    hash_element *elem = hash_get(model->index_to_target, word_index);
    if (elem == NULL) {
        elem = hash_add(model->index_to_target, word_index, model->num_targets++);
        if (model->num_targets >= model->max_targets)
        {
            unsigned_array **new_targets = (unsigned_array **)malloc(sizeof(unsigned_array *)*model->max_targets*2);
            memcpy(new_targets,model->targets,sizeof(unsigned_array *)*model->max_targets);
            for(int i=model->max_targets;i<model->max_targets*2;i++) {
            	new_targets[i] = unsigned_array_new(32);
            }
            free(model->targets);
            model->targets = new_targets;
            //new_targets+=1;
            model->max_targets *= 2;
        }
        model->targets[elem->value] = unsigned_array_new(32);
        unsigned_array_set(model->assignments, elem->value, model->num_categories++);
    }
    return (model->targets[elem->value]);
}

OCW *static_ocw_model;
progressbar *static_progress;
int document_index;
void OCW_each_document(unsigned int target, unsigned int *words, unsigned int length)
{
    if(length < 1)
    	return;
    unsigned_array *representation = OCW_get_target(static_ocw_model, target);
    for (int i=0; i<length; i++) {
        unsigned_array_add(representation, words[i], 1);
    }

    int t_index = hash_get(static_ocw_model->index_to_target, target)->value;
    double best_distance = 0.0;
    int best = target;
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        double distance = cosine_ua(static_ocw_model->targets[t_index], static_ocw_model->targets[i]);
//        double distance = manhattan(static_ocw_model->targets[t_index], static_ocw_model->targets[i]);
        double_matrix_set(static_ocw_model->distances,i,t_index,distance);
        double_matrix_set(static_ocw_model->distances,t_index,i,distance);
    }

//    OCW_train_step(static_ocw_model);
    for (int j=0; j<model->num_targets; j++) {
        if (target != j) {
            double pairwise = double_matrix_get(model->distances,target,j);
            if (pairwise > distance) {
                distance = pairwise;
                closest = j;
            }
        }
    }
    if (distance > 0.0 && closest != i) {
        unsigned_array_set(model->assignments, i, unsigned_array_get(model->assignments, closest));
    }

    progressbar_inc(static_progress);
    document_index++;
}

// Helper function for shuffling an array.
// http://benpfaff.org/writings/clc/shuffle.html
void OCW_shuffle(unsigned int *array, unsigned int n)
{
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

/* Do a single training step for an OCW model. 
 * 
 *   1. Assign a unique class to any new items.
 *   2. Compute the pairwise distance between all items.
 *   3. Run one iteration of the CW algorithm.
 */
void OCW_train_step(OCW *model)
{
    // Randomize order
    unsigned int *order = (unsigned int *)malloc(sizeof(unsigned int)*model->num_targets);
    for (int i=0; i<model->num_targets; i++) {
        order[i] = i;
    }
    OCW_shuffle(order, model->num_targets);
    
    double distance;
    int closest;
    int i;
    for (int ix=0; ix<model->num_targets; ix++) {
        i = order[ix];
        closest = i;
        distance = -2.0;

        for (int j=0; j<model->num_targets; j++) {
            if (i != j) {
                double pairwise = double_matrix_get(model->distances,i,j);
                if (pairwise > distance) {
                    distance = pairwise;
                    closest = j;
                }
            }
        }
        if (distance > 0.0 && closest != i) {
            unsigned_array_set(model->assignments, i, unsigned_array_get(model->assignments, closest));
        }
    }
    free(order);
}

void OCW_train(OCW *model)
{
    static_ocw_model = model;
    static_progress = progressbar_new("Building representations", model->corpus->document_count);
    document_index = 0;
    target_corpus_each_document(model->corpus, &OCW_each_document);
    progressbar_finish(static_progress);
    /*
    int cw_step = 1;
    progressbar *cw = progressbar_new("Clustering", cw_step);
    for (int i=0; i<cw_step; i++) {
        OCW_train_step(static_ocw_model);
        progressbar_inc(cw);
    }
    progressbar_finish(cw);
     */
    static_ocw_model = NULL;
}

void OCW_dump(OCW *model)
{
    /*
    int *counts = (int *)malloc(sizeof(int)*model->num_targets);
    memset(counts,0x0,model->num_targets*sizeof(int));
    for (int i=0; i<model->num_targets; i++) {
        counts[unsigned_array_get(model->assignments,i)]++;
    }
    unsigned int total = 0;
    for (int i=0; i<model->num_targets; i++) {
        if (counts[i] > 0) {
            total++;
        }
    }
    */
    char *wordmap_f = (char *)malloc(strlen(model->corpus_filename)+9);
    sprintf(wordmap_f,"%s.wordmap",model->corpus_filename);
    WordMap_dump(model->corpus->wordmap, wordmap_f);
    
    for (int i=0; i<model->assignments->size; i++) {
        printf("%d %d\n",hash_reverse_lookup(model->index_to_target, i)->key,unsigned_array_get_zero(model->assignments, i));
    }
}

