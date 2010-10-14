/*
 *  focw.c
 *  corncob
 *
 *  Created by Trevor Fountain on 9/8/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */
 
//#define DEBUG
#define PPMI

#include "focw.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

int main(int argc, char **argv)
{
	if(argc < 3) {
		printf("focw expects 2-3 argument, recieved %d\n",argc-1);
		printf("\nUsage: focw <corpus> <threshold> [interval]\n");
		return 1;
	}
    
    int interval = -1;
    if (argc >= 4) {
        interval = atoi(argv[3]);
    }
    
    srand(time(NULL));
    OCW *model = OCW_new(argv[1],atof(argv[2]),interval);
    OCW_save_wordmap(model);
    OCW_train(model);
    OCW_save_categorization(model);
    OCW_free(model);
    
	return 0;
}

OCW *OCW_new(char *filename, double threshold, int interval)
{
    OCW *model = (OCW *)malloc(sizeof(OCW));
    model->threshold = threshold;
    model->output_every_index = interval;
    model->corpus = target_corpus_new(filename);
    model->corpus_filename = filename;
    model->document_index = 0;
    model->num_targets = 0;
    model->max_targets = 2000;
    model->num_categories = 0;
    model->assignments = unsigned_array_new(2000);
    model->targets = (ct_hash **)malloc(sizeof(ct_hash *)*model->max_targets);
    for (int i=0; i<model->max_targets; i++) {
        model->targets[i] = NULL;
    }
    model->wordmap_to_target = hash_new(2000);
    
    // Distances
    model->distances = double_matrix_new(500, 500, 0.0);
    
    // PPMI
    model->f_xx = 0;
    model->context_counts = hash_new(2000);
    
    // Output interim counts and ppmi vectors?
    model->output_counts = 0;
    model->output_meanings = 0;
    
    return model;
}

void OCW_free(OCW *model)
{
    for (int i=0; i<model->num_targets; i++) {
        if (model->targets[i] != NULL) {
            hash_free(model->targets[i]);
        }
    }
    free(model->targets);
    unsigned_array_free(model->assignments);
    hash_free(model->wordmap_to_target);
    hash_free(model->context_counts);
    free(model);
}

OCW *static_ocw_model;
progressbar *static_progress;
void OCW_each_document(unsigned int target, unsigned int *words, unsigned int length)
{
    if(length < 1)
    	return;
    
    // Look up the target index for this +target+
    hash_element *element = hash_get(static_ocw_model->wordmap_to_target, target);
    int index = 0; // +index+ is the index in model->targets for this target
    if (element != NULL) {
        // Success! We've seen this word before.
        index = element->value;
    } else {
        // +target+ is a new word, so add it.
        index = static_ocw_model->num_targets++;
        int category = static_ocw_model->num_categories++;
        hash_add(static_ocw_model->wordmap_to_target, target, index);
        unsigned_array_set(static_ocw_model->assignments, index, category);
        static_ocw_model->targets[index] = hash_new(32);
    }
    
    // Update the frequency counts for this target
    for (int i=0; i<length; i++) {
        //model->targets[i] is f_ij, where i is the target word and j is the context word
        hash_update(static_ocw_model->targets[index], words[i], 1);
        static_ocw_model->f_xx++;
        
        // Also update the frequency count for this context word
        hash_update(static_ocw_model->context_counts, words[i], 1);
    }
    
    // Compute PPMI vector for this target
    double_hash *target_ppmi = OCW_ppmi(static_ocw_model,index);
    
    // Update distance matrix
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (i != index) { // Don't compute distance between target and itself
            double_hash *other_ppmi = OCW_ppmi(static_ocw_model, i);
            double distance = double_hash_cosine(target_ppmi,other_ppmi);
            double_hash_free(other_ppmi);
            double_matrix_set(static_ocw_model->distances, index, i, distance);
            double_matrix_set(static_ocw_model->distances, i, index, distance);
        }
    }
    
    // Find nearest neighbors that need updating
    double best_distance = 0.0;
    int best_index = -1;
    double match_distance = static_ocw_model->threshold;
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (index != i) {
            double distance = double_matrix_get_zero(static_ocw_model->distances, index, i);
            if (distance > match_distance && distance > best_distance) {
                best_index = i;
                best_distance = distance;
            } else if (distance > 0.0 && unsigned_array_get(static_ocw_model->assignments, index) == unsigned_array_get(static_ocw_model->assignments, i)) {
                // If they're not closely related but share a category, split.
                unsigned_array_set(static_ocw_model->assignments,index,static_ocw_model->num_categories++);
                unsigned_array_set(static_ocw_model->assignments,i,static_ocw_model->num_categories++);
            }
        }
    }
    if (best_index >= 0) {
        int i = index,j = best_index;
        if (rand()/100 > 50) {
            i = best_index;
            j = index;
        }
        unsigned_array_set(static_ocw_model->assignments, i, unsigned_array_get(static_ocw_model->assignments, j));    
    }
    
    double_hash_free(target_ppmi);
    
    if (static_ocw_model->output_every_index > 0 && static_ocw_model->document_index % static_ocw_model->output_every_index == 0 && static_ocw_model->document_index != 0) {
        progressbar_finish(static_progress);
        OCW_save_categorization(static_ocw_model);
        if(static_ocw_model->output_meanings) {
        	OCW_save_meanings(static_ocw_model);
        }
        if(static_ocw_model->output_counts) {
        	OCW_save_representations(static_ocw_model);
        }
        static_progress = progressbar_new("Training", static_ocw_model->output_every_index);
    }
    progressbar_inc(static_progress);
    static_ocw_model->document_index++;
}

void OCW_train(OCW *model)
{
    static_ocw_model = model;
    static_progress = progressbar_new("Training", model->output_every_index);
    target_corpus_each_document(model->corpus, &OCW_each_document);
    progressbar_finish(static_progress);
    static_ocw_model = NULL;
}

void OCW_save_wordmap(OCW *model)
{
    char *wordmap_f = (char *)malloc(strlen(model->corpus_filename)+9);
    sprintf(wordmap_f,"%s.wordmap",model->corpus_filename);
    WordMap_dump(model->corpus->wordmap, wordmap_f);
    
    free(wordmap_f);
}

void OCW_save_categorization(OCW *model)
{
    char save_f[60];
    char threshold_f[10];
    sprintf(threshold_f,"%.2f",model->threshold);
    threshold_f[1] = '_';
    if (model->output_every_index > 0) {
        sprintf(save_f,"%s.%d.%s.focw",model->corpus_filename,model->document_index,threshold_f);
        printf("Saving partial categorization %s\n",save_f);
    } else {
        sprintf(save_f,"%s.focw",model->corpus_filename);
        printf("Saving final categorization %s\n",save_f);
    }
    FILE *fout = fopen(save_f,"w");
    for (int i=0; i<model->num_targets; i++) {
        int index = hash_reverse_lookup(model->wordmap_to_target, i)->key;
        fprintf(fout,"%d\t%d\n",index,unsigned_array_get(model->assignments, i));
    }
    fclose(fout);
}

FILE *static_save_file;
void OCW_save_representations(OCW *model)
{
  char save_f[60];
  char threshold_f[10];
  sprintf(threshold_f,"%.2f",model->threshold);
  threshold_f[1] = '_';
  sprintf(save_f,"%s.%d.%s.reps",model->corpus_filename,model->document_index,threshold_f);
  printf("Saving raw counts %s\n",save_f);
	static_save_file = fopen(save_f,"w");
	for(int i=0;i<model->max_targets;i++) {
		if(model->targets[i] != NULL) {
      hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
      char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
      hash_fprint_labeled(static_save_file,model->targets[i],element_label,model->corpus->wordmap);
		} else {
			break;
		}
	}
	fclose(static_save_file);
}

void OCW_save_meanings(OCW *model)
{
	char save_f[60];
	char threshold_f[10];
	sprintf(threshold_f,"%.2f",model->threshold);
	threshold_f[1] = '_';
	sprintf(save_f,"%s.%d.%s.ppmi",model->corpus_filename,model->document_index,threshold_f);
  printf("Saving PPMI vectors %s\n",save_f);
	static_save_file = fopen(save_f,"w");
	for(int i=0;i<model->max_targets;i++) {
		if(model->targets[i] != NULL) {
			double_hash *ppmi = OCW_ppmi(model,i);
      hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
      char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
      double_hash_fprint_labeled(static_save_file,ppmi,element_label,model->corpus->wordmap);
		} else {
			break;
		}
	}
	fclose(static_save_file);
}

void swap_pair(Pair *a, Pair *b)
{
    Pair temp;
    temp.a = a->a;
    temp.b = a->b;
    a->a = b->a;
    a->b = b->b;
    b->a = temp.a;
    b->b = temp.b;
}

void array_shuffle(Pair *array,unsigned int size)
{
    for (int i=0; i<size; i++) {
        swap_pair(&array[i], &array[rand()%size]);
    }
}

double_hash *static_ppmi;
unsigned int OCW_static_f_ix;
void OCW_ppmi_helper(hash_element *element)
{
    // element = {key => context_word, value => f_ij}
    unsigned int f_ij = element->value;
    
    hash_element *context_element = hash_get(static_ocw_model->context_counts, element->key);
    unsigned int f_xj = context_element->value;
    
    unsigned int f_xx = static_ocw_model->f_xx;
    unsigned int f_ix = OCW_static_f_ix;
    
    double p_ij = (double)f_ij / f_xx;
    double p_ix = (double)f_ix / f_xx;
    double p_xj = (double)f_xj / f_xx;
    
    double ppmi_ij = log(p_ij/(p_ix * p_xj));
    double_hash_add(static_ppmi, element->key, 
#ifdef PPMI
                    (ppmi_ij < 0 ? 0: ppmi_ij)
#else
                    ppmi_ij
#endif
                    );
}

double_hash *OCW_ppmi(OCW *model,int index)
{
    double_hash *target_ppmi = double_hash_new(10);
    OCW_static_f_ix = model->targets[index]->sum;
    
    static_ppmi = target_ppmi;
    hash_foreach(model->targets[index], &OCW_ppmi_helper);
    
    return target_ppmi;
}
