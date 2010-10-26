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

void print_usage(const char *error)
{
    printf("Usage: focw <corpus> <interval> [update method]\n");
    printf("\n[update method] may be one of:\n");
    printf("\ttop\n");
    printf("\tnearest\n");
    printf("\tvote <threshold>\n");
    printf("\n%s\n",error);
    
    exit(1);
}

void usage()
{
    print_usage("");
}

int main(int argc, char **argv)
{
    OCW *model;
    if (argc == 2) {
        // Resume
        char *filename = argv[1];
        char focw_filename[60];
        strcpy(focw_filename,filename);
        char corpus[30];
        strcpy(corpus,strtok(filename,"."));
        int document_index = atoi(strtok(NULL,"."));
        char alg[20];
        strcpy(alg,strtok(NULL,"."));
        
        // TODO: parameterize this, somehow.
        int interval = 2500;
        
        double threshold = 0.0;
        if (alg[strlen(alg)-3] == '_') {
            char threshold_s[10];
            strcpy(threshold_s,alg+5);
            threshold = atof(threshold_s);
            alg[4] = '\0';
        }
        
        algorithm method = TOP;
        if (!strcmp(alg,"top")) {
            method = TOP;
        } else if (!strcmp(alg,"nearest")) {
            method = NEAREST;
        } else if (!strcmp(alg,"vote")) {
            method = VOTE;
        }
        
        model = OCW_resume(focw_filename,corpus,document_index,method,threshold,interval);
    } else {
        if (argc < 3) {
            if (argc < 2) {
                print_usage("No target corpus or output interval specified");
            } else {
                print_usage("No output interval specified");
            }
        }

        // Target corpus
        char *corpus = argv[1];
        
        int interval = atoi(argv[2]);
        
        // Update algorithm. Some algorithms take additional parameters...
        algorithm update_method = TOP;
        double threshold = 0.0;
        if (argc > 3) {
            const char *method = argv[3];
            if (!strcmp(method,"top")) {
                update_method = TOP;
            } else if (!strcmp(method,"nearest")) {
                update_method = NEAREST;
            } else if (!strcmp(method,"vote")) {
                update_method = VOTE;
                if (argc < 5) {
                    print_usage("'top' requires an additional parameter");
                }
                threshold = atof(argv[4]);
            } else {
                print_usage("Too many arguments!");
            }
        } else {
            printf("No update method specified, assuming top...\n");
        }
        
        model = OCW_new(corpus,update_method,threshold,interval);
        OCW_save_wordmap(model);
    }
    OCW_train(model);
    OCW_save_categorization(model);
    printf("OCW trial (%s %f %d) complete!\n",model->corpus_filename,model->threshold,model->output_every_index);
    
	return 0;
}

OCW *OCW_new(char *filename, algorithm alg, double threshold, int interval)
{
    OCW *model = (OCW *)malloc(sizeof(OCW));
    model->update_method = alg;
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
    
    return model;
}

OCW *OCW_resume(char *focw_filename, char *corpus_filename, int document_index, algorithm alg, double threshold, int interval)
{
    printf("Resuming trial (%s %d %d %f %d) from %s\n",corpus_filename,document_index,alg,threshold,interval,focw_filename);
    
    OCW *model = (OCW *)malloc(sizeof(OCW));
    model->update_method = alg;
    model->threshold = threshold;
    model->output_every_index = interval;
    model->corpus = target_corpus_new(corpus_filename);
    model->corpus_filename = corpus_filename;
    model->document_index = 0;
    model->skip_documents = document_index;
    model->max_targets = 2000;
    model->assignments = unsigned_array_new(2000);
    model->targets = (ct_hash **)malloc(sizeof(ct_hash *)*model->max_targets);
    for (int i=0; i<model->max_targets; i++) {
        model->targets[i] = NULL;
    }
    model->num_targets = 0;
    model->num_categories = 0;
    model->distances = double_matrix_new(500, 500, 0.0);
    model->f_xx = 0;
    model->context_counts = hash_new(2000);
    
    // Load targets, categories, and assignments
    FILE *fin = fopen(focw_filename,"r");
    char *line = (char *)malloc(sizeof(char)*80);
    while (!feof(fin)) {
        fgets(line,80,fin);
        if (strlen(line) <= 1 || strstr(line,"\t") == NULL) {
            break;
        }
        char target_s[10];
        strcpy(target_s,strtok(line,"\t"));
        int target = atoi(target_s);
        char category_s[10];
        char *v = strtok(NULL,"\t");
        if (v == NULL) {
            break;
        }
        strcpy(category_s,v);
        int category = atoi(category_s);
        
        unsigned_array_set(model->assignments, target, category);
        model->num_targets++;
        if (category > model->num_categories) {
            model->num_categories = category;
        }
    }
    free(line);
    fclose(fin);
    
    // Load wordmap_to_target
    char *wordmap_to_target_filename = (char *)malloc(sizeof(char)*strlen(focw_filename)+4);
    strcpy(wordmap_to_target_filename,focw_filename);
    wordmap_to_target_filename[strlen(wordmap_to_target_filename)-4] = '\0';
    strcat(wordmap_to_target_filename,"wmt");
    model->wordmap_to_target = hash_load(wordmap_to_target_filename);
    free(wordmap_to_target_filename);
    
    // Load representations    
    char *reps_filename = (char *)malloc(sizeof(char)*strlen(focw_filename)+4);
    strcpy(reps_filename,focw_filename);
    reps_filename[strlen(reps_filename)-4] = '\0';
    strcat(reps_filename,"reps");
    fin = fopen(reps_filename,"r");
    int line_length = 100000;
    line = (char *)malloc(sizeof(char)*line_length);
    int line_index = 0;
    while (!feof(fin)) {
        fgets(line,line_length,fin);
        
        if (strchr(line,'(') == NULL) {
            break;
        }
        
        if (line[strlen(line)-1] != '\n') {
            printf("Error in resume:\n");
            printf("(%s): line %d too long, quitting.\n",reps_filename,line_index);
            printf("<<%s>>\n",line);
            exit(1);
        }
        
        char target_s[80];
        char *tok = strtok(line,"<");
        strcpy(target_s,tok);
        target_s[strlen(target_s)-1] = '\0';
        int target_wmi = WordMap_index(model->corpus->wordmap,target_s);
        int target_index = hash_get(model->wordmap_to_target,target_wmi)->value;
        if (model->targets[target_index] == NULL) {
            model->targets[target_index] = hash_new(2000);
        }
        while (tok != NULL) {
            char index_s[10];
            tok = strtok(NULL,",");
            if (tok[1] == '>') {
                break;
            }
            strcpy(index_s,tok+1);
            
            char value_s[10];
            char *v = strrchr(index_s,'(');
            if (v != NULL) {
                strcpy(value_s,v);
            } else {
                break;
            }

            char fvalue_s[10];
            strcpy(fvalue_s,&value_s[1]);
            fvalue_s[strlen(fvalue_s)-1] = '\0';
            
            index_s[strlen(index_s)-strlen(fvalue_s)-3] = '\0';
            int key = atoi(index_s);
            int value = atoi(fvalue_s);
            
            hash_update(model->targets[target_index], key, value);
            model->f_xx += value;
            hash_update(model->context_counts, key, value);
        }
        
        line_index++;
    }
    free(reps_filename);
    
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
    if (static_ocw_model->skip_documents > 0) {
        static_ocw_model->skip_documents--;
        static_ocw_model->document_index++;
        return;
    }
    
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
    
    // Find highest ranked category in neighboorhood
    double_hash *class_distances = double_hash_new(static_ocw_model->num_categories/3);
    int best_index = index;
    double best_distance = 0.0;
    for (int i=0; i<static_ocw_model->num_targets; i++) {
        if (index != i) {
            double distance = double_matrix_get_zero(static_ocw_model->distances, index, i);
            if (distance > 0) {
                unsigned int category = unsigned_array_get(static_ocw_model->assignments, i);
                double_hash_add(class_distances, category, distance);
                
                if (distance > best_distance) {
                    best_index = i;
                    best_distance = distance;
                }
            }
        }
    }
    
    // Update class assignment based on specified update method (see CW manual for more info)
    if (class_distances->size > 0) {
        int best_key;
        double best_value;
        switch (static_ocw_model->update_method) {
            case NEAREST:
                unsigned_array_set(static_ocw_model->assignments, index, unsigned_array_get(static_ocw_model->assignments, best_index));
                break;
            case TOP:
                unsigned_array_set(static_ocw_model->assignments, index, double_hash_largest_key(class_distances));
                break;
            case VOTE:
                best_key = double_hash_largest_key(class_distances);
                best_value = double_hash_get(class_distances, best_key)->value;
                if (best_value > static_ocw_model->threshold) {
                    unsigned_array_set(static_ocw_model->assignments, index, best_key);
                }
                break;
            default:
                break;
        }
    }
    double_hash_free(class_distances);
    
    double_hash_free(target_ppmi);
    
    if (static_ocw_model->output_every_index > 0 && static_ocw_model->document_index % static_ocw_model->output_every_index == 0 && static_ocw_model->document_index != 0) {
        progressbar_finish(static_progress);
        OCW_save_categorization(static_ocw_model);
        OCW_save_representations(static_ocw_model);
        OCW_save_target_wordmap(static_ocw_model);
        static_progress = progressbar_new("Training", static_ocw_model->output_every_index);
    }
    progressbar_inc(static_progress);
    static_ocw_model->document_index++;
}

void OCW_train(OCW *model)
{
    srand(time(NULL));
    
    static_ocw_model = model;
    static_progress = progressbar_new("Training", model->output_every_index);
    target_corpus_each_document(model->corpus, &OCW_each_document);
    progressbar_finish(static_progress);
    static_ocw_model = NULL;
}

const char *OCW_method_str_map(OCW *model)
{
    char *str;
    switch (model->update_method) {
        case TOP:
            return "top";
        case NEAREST:
            return "nearest";
        case VOTE:
            str = (char *)malloc(sizeof(char) * 10);
            str[9] = '\0';
            sprintf(str,"vote_%1.2f",model->threshold);
            str[6] = '_';
            return str;
        default:
            return "UNKNOWN";
    }
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
    sprintf(save_f,"%s.%d.%s.focw",model->corpus_filename,model->document_index,OCW_method_str_map(model));
    printf("Saving categorization %s\n",save_f);
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
    sprintf(save_f,"%s.%d.%s.reps",model->corpus_filename,model->document_index,OCW_method_str_map(model));
    printf("Saving raw counts %s\n",save_f);
	static_save_file = fopen(save_f,"w");
	for(int i=0;i<model->max_targets;i++) {
		if(model->targets[i] != NULL) {
            hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
            char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
            hash_fprint(static_save_file,model->targets[i],element_label);
		} else {
			break;
		}
	}
	fclose(static_save_file);
}

void OCW_save_target_wordmap(OCW *model)
{
    char save_f[60];
    sprintf(save_f,"%s.%d.%s.wmt",model->corpus_filename,model->document_index,OCW_method_str_map(model));
    printf("Saving wordmap %s\n",save_f);
    hash_save(model->wordmap_to_target, save_f);
}

void OCW_save_meanings(OCW *model)
{
//	char save_f[60];
//	char threshold_f[10];
//	sprintf(threshold_f,"%.2f",model->threshold);
//	threshold_f[1] = '_';
//	sprintf(save_f,"%s.%d.%s.ppmi",model->corpus_filename,model->document_index,threshold_f);
//  printf("Saving PPMI vectors %s\n",save_f);
//	static_save_file = fopen(save_f,"w");
//	for(int i=0;i<model->max_targets;i++) {
//		if(model->targets[i] != NULL) {
//			double_hash *ppmi = OCW_ppmi(model,i);
//      hash_element *rev = hash_reverse_lookup(model->wordmap_to_target, i);
//      char *element_label = WordMap_reverse_lookup(model->corpus->wordmap, rev->key);
//      double_hash_fprint_labeled(static_save_file,ppmi,element_label,model->corpus->wordmap);
//		} else {
//			break;
//		}
//	}
//	fclose(static_save_file);
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
