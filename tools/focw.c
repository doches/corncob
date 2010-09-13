/*
 *  focw.c
 *  corncob
 *
 *  Created by Trevor Fountain on 9/8/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "focw.h"

int main(int argc, char **argv)
{
	if(argc != 3) {
		printf("focw expects 2 argument, recieved %d\n",argc-1);
		printf("\nUsage: focw <corpus> <threshold>\n");
		return 1;
	}
    
    OCW *model = OCW_new(argv[1],atof(argv[2]));
    OCW_save_wordmap(model);
    OCW_train(model);
    OCW_free(model);
    
	return 0;
}


OCW *OCW_new(char *filename,double threshold)
{
    OCW *model = (OCW *)malloc(sizeof(OCW));
    model->threshold = threshold;
    model->corpus = target_corpus_new(filename);
    model->corpus_filename = filename;
    model->document_index = 0;
    model->num_targets = 0;
    model->max_targets = 512;
    model->num_categories = 0;
    model->assignments = unsigned_array_new(512);
    model->targets = (ct_hash **)malloc(sizeof(ct_hash *)*model->max_targets);
    for (int i=0; i<model->max_targets; i++) {
        model->targets[i] = NULL;
    }
    model->wordmap_to_target = hash_new(512);
    
    return model;
}

void OCW_free(OCW *model)
{
    target_corpus_free(model->corpus);
    for (int i=0; i<model->num_targets; i++) {
        hash_free(model->targets[i]);
    }
    
    free(model->targets);
    unsigned_array_free(model->assignments);
    hash_free(model->wordmap_to_target);
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
    if (element != NULL) { // Success! We've seen this word before.
        index = element->value;
    } else {               // +target+ is a new word, so add it.
        index = static_ocw_model->num_targets++;
        static_ocw_model->targets[index] = hash_new(32);
        hash_add(static_ocw_model->wordmap_to_target, target, index);
        unsigned_array_set(static_ocw_model->assignments, index, index);
    }
    
    // Update the representation for the target
    for (int i=0; i<length; i++) {
        hash_add(static_ocw_model->targets[index], words[i], 1);
    }
    
    // Update category assignment for target
    double best_distance = 0.0;
    int best_index = index;
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (i != index) { // Don't compute distance between target and itself
            double distance = hash_cosine(static_ocw_model->targets[index],static_ocw_model->targets[i]);
            if (distance > best_distance) {
                best_distance = distance;
                best_index = i;
            }
        }
    }
    if (best_index != index && best_distance >= static_ocw_model->threshold) {
        unsigned_array_set(static_ocw_model->assignments, index, unsigned_array_get(static_ocw_model->assignments, best_index));
    }
    
    if (static_ocw_model->document_index++ % 10000 == 0 && static_ocw_model->document_index != 0) {
        OCW_save_categorization(static_ocw_model);
    }
    progressbar_inc(static_progress);
}

void OCW_train(OCW *model)
{
    static_ocw_model = model;
    static_progress = progressbar_new("Training", model->corpus->document_count);
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
    char *save_f = (char *)malloc(sizeof(char)*(strlen(model->corpus_filename)+6));
    char *threshold_f = (char *)malloc(sizeof(char)*6);
    sprintf(threshold_f,"%.2f",model->threshold);
    threshold_f[1] = '_';
    sprintf(save_f,"%s.%dk.%s.focw",model->corpus_filename,model->document_index/1000,threshold_f);
    FILE *fout = fopen(save_f,"w");
    for (int i=0; i<model->num_targets; i++) {
        int index = hash_reverse_lookup(model->wordmap_to_target, i)->key;
        fprintf(fout,"%d\t%d\n",index,unsigned_array_get(model->assignments, i));
    }
    fclose(fout);
    
    free(save_f);
    free(threshold_f);
}