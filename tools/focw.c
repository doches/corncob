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
    
    // PPMI
    model->f_xx = 0;
    model->context_counts = hash_new(2000);
    
    // Output interim counts and ppmi vectors?
    model->output_counts = 1;
    model->output_meanings = 1;
    
    return model;
}

void OCW_free(OCW *model)
{
    target_corpus_free(model->corpus);
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
//    hash_sanity_check(static_ocw_model->wordmap_to_target);
    hash_element *element = hash_get(static_ocw_model->wordmap_to_target, target);
    int index = 0; // +index+ is the index in model->targets for this target
    if (element != NULL) { // Success! We've seen this word before.
        index = element->value;
    } else {               // +target+ is a new word, so add it.
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
    
//    hash_sanity_check(static_ocw_model->wordmap_to_target);
    // Compute PPMI vector for this target
    double_hash *target_ppmi = OCW_ppmi(static_ocw_model,index);
#ifdef DEBUG
    hash_element *revx = hash_reverse_lookup(static_ocw_model->wordmap_to_target, index);
    char *elementx_label = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, revx->key);
    double_hash_print_labeled(target_ppmi,elementx_label,static_ocw_model->corpus->wordmap);
//    hash_printx(static_ocw_model->targets[index],elementx_label);
#endif
    
    // Update distance matrix
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (i != index) { // Don't compute distance between target and itself
            double_hash *other_ppmi = OCW_ppmi(static_ocw_model, i);
            double distance = double_hash_cosine(target_ppmi,other_ppmi);
#ifdef DEBUG
            double_hash *element_intersect = double_hash_intersection(other_ppmi, target_ppmi);
            if (element_intersect->size > 0) {
                int last_printed = 0;
                printf("  ( %.4f )  %n",distance,&last_printed);
                hash_element *rev = hash_reverse_lookup(static_ocw_model->wordmap_to_target, i);
                char *element_label = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, rev->key);
                double_hash_print_labeled(element_intersect,element_label,static_ocw_model->corpus->wordmap);
            }
#endif
            double_hash_free(other_ppmi);
            static_ocw_model->distances[index][i] = distance;
            static_ocw_model->distances[i][index] = distance;
        }
    }
    
    // Find nearest neighbors that need updating
    Pair updates[5000];
    unsigned int num_updates = 0;
    double match_distance = static_ocw_model->threshold;
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (index != i) {
            double distance = static_ocw_model->distances[index][i];
            if (distance > match_distance) {
                // if two words are closely related, share a category
                updates[num_updates].a = i;
                updates[num_updates].b = index;
                num_updates++;
                updates[num_updates].b = i;
                updates[num_updates].a = index;
                num_updates++;
            } else if (distance > 0.0 && unsigned_array_get(static_ocw_model->assignments, index) == unsigned_array_get(static_ocw_model->assignments, i)) {
                // If they're not closely related but share a category, split one off.
                int split = index;
                int from = i;
                if (rand() < 0.5) {
                    split = i;
                    from = index;
                }
#ifdef DEBUG
                char *a = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, hash_reverse_lookup(static_ocw_model->wordmap_to_target, split)->key);
                char *b = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, hash_reverse_lookup(static_ocw_model->wordmap_to_target, from)->key);
                printf("%s (%d) <- %d (too dissimilar from %s (%d))\n",a,unsigned_array_get(static_ocw_model->assignments,split),static_ocw_model->num_categories,b,unsigned_array_get(static_ocw_model->assignments,from));;
#endif
                unsigned_array_set(static_ocw_model->assignments,split,static_ocw_model->num_categories++);
            }
        }
    }
    
#ifdef DEBUG
    printf("\n");
#endif
    
    // Do CW
    if(num_updates > 0) {
      array_shuffle(updates,num_updates);
      for (int i=0; i<num_updates; i++) {
#ifdef DEBUG
          char *a = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, hash_reverse_lookup(static_ocw_model->wordmap_to_target, updates[i].a)->key);
          char *b = WordMap_reverse_lookup(static_ocw_model->corpus->wordmap, hash_reverse_lookup(static_ocw_model->wordmap_to_target, updates[i].b)->key);
          printf("%s (%d) <- %s (%d)\n",a,unsigned_array_get(static_ocw_model->assignments, updates[i].a),b,unsigned_array_get(static_ocw_model->assignments, updates[i].b));
#endif
          unsigned_array_set(static_ocw_model->assignments, updates[i].a, unsigned_array_get(static_ocw_model->assignments, updates[i].b));
      }
    }
    double_hash_free(target_ppmi);
    
#ifdef DEBUG
    printf("\n");
#endif
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
	static_save_file = fopen(save_f,"w");
	for(int i=0;i<model->max_targets;i++) {
		if(model->targets[i] != NULL) {
      hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
      char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
      hash_fprint_labeled(static_save_file,model->targets[i],element_label,model->corpus->wordmap);
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
	static_save_file = fopen(save_f,"w");
	for(int i=0;i<model->max_targets;i++) {
		if(model->targets[i] != NULL) {
			double_hash *ppmi = OCW_ppmi(model,i);
      hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
      char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
      double_hash_fprint_labeled(static_save_file,ppmi,element_label,model->corpus->wordmap);
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
