//#define DEBUG
#define PPMI

#include "cw.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

void print_usage(const char *error)
{
    printf("Usage: cw <corpus> [update method]\n");
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
    CW *model;
    if (argc < 2) {
        print_usage("No target corpus specified");
    }

    // Target corpus
    char *corpus = argv[1];
    
    // Update algorithm. Some algorithms take additional parameters...
    algorithm update_method = TOP;
    double threshold = 0.0;
    if (argc > 3) {
        const char *method = argv[2];
        if (!strcmp(method,"top")) {
            update_method = TOP;
        } else if (!strcmp(method,"nearest")) {
            update_method = NEAREST;
        } else if (!strcmp(method,"vote")) {
            update_method = VOTE;
            if (argc < 5) {
                print_usage("'top' requires an additional parameter");
            }
            threshold = atof(argv[3]);
        } else {
            print_usage("Too many arguments!");
        }
    } else {
        printf("No update method specified, assuming top...\n");
    }
    
    model = CW_new(corpus,update_method,threshold);
    CW_save_wordmap(model);
    CW_train(model);
    CW_save_categorization(model);
    printf("CW trial (%s %f) complete!\n",model->corpus_filename,model->threshold);
    
	return 0;
}

CW *CW_new(char *filename, algorithm alg, double threshold)
{
    CW *model = (CW *)malloc(sizeof(CW));
    model->update_method = alg;
    model->threshold = threshold;
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
    
    for (int i=0; i<1000; i++) {
        model->cached_ppmi[i] = NULL;
    }
    
    return model;
}

void CW_free(CW *model)
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

CW *static_cw_model;
progressbar *static_progress;
void CW_each_document(unsigned int target, unsigned int *words, unsigned int length)
{
    if (static_cw_model->skip_documents > 0) {
        static_cw_model->skip_documents--;
        static_cw_model->document_index++;
        return;
    }
    
    if(length < 1)
    	return;
    
    // Look up the target index for this +target+
    hash_element *element = hash_get(static_cw_model->wordmap_to_target, target);
    int index = 0; // +index+ is the index in model->targets for this target
    if (element != NULL) {
        // Success! We've seen this word before.
        index = element->value;
    } else {
        // +target+ is a new word, so add it.
        index = static_cw_model->num_targets++;
        int category = static_cw_model->num_categories++;
        hash_add(static_cw_model->wordmap_to_target, target, index);
        unsigned_array_set(static_cw_model->assignments, index, category);
        static_cw_model->targets[index] = hash_new(32);
    }
    
    // Update the frequency counts for this target
    for (int i=0; i<length; i++) {
        //model->targets[i] is f_ij, where i is the target word and j is the context word
        hash_update(static_cw_model->targets[index], words[i], 1);
        static_cw_model->f_xx++;
        
        // Also update the frequency count for this context word
        hash_update(static_cw_model->context_counts, words[i], 1);
    }
    
    progressbar_inc(static_progress);
    static_cw_model->document_index++;
}

void CW_iteration(CW *model)
{
    // Build a randomly-ordered array of target word indices
    int *target_indices = (int *)malloc(sizeof(int)*model->num_targets);
    for (int i=0; i<model->num_targets; i++) {
        target_indices[i] = 0;
    }
    
    for (int i=0; i<model->num_targets; i++) {
        int a = rand()%model->num_targets;
        int b = rand()%model->num_targets;
        
        int temp = target_indices[a];
        target_indices[a] = target_indices[b];
        target_indices[b] = temp;
    }
    
    for (int index=0; index<model->num_targets; index++) {
        // Compute PPMI vector for this target
        if (model->cached_ppmi[index] != NULL) {
            double_hash_free(model->cached_ppmi[index]);
        }
        model->cached_ppmi[index] = CW_ppmi(model,index);
        double_hash *target_ppmi = model->cached_ppmi[index];
        
        // Update distance matrix
        for (int i=0; i<model->num_targets; i++) {
            if (i != index) { // Don't compute distance between target and itself
                double_hash *other_ppmi = model->cached_ppmi[i];
                if (other_ppmi == NULL) {
                    model->cached_ppmi[i] = CW_ppmi(model, i);
                    other_ppmi = model->cached_ppmi[i];
                }
                
                double distance = double_hash_cosine(target_ppmi,other_ppmi);
                double_matrix_set(model->distances, index, i, distance);
                double_matrix_set(model->distances, i, index, distance);
            }
        }
        
        // Find highest ranked category in neighboorhood
        double_hash *class_distances = double_hash_new(50);
        int best_index = index;
        double best_distance = 0.0;
        for (int i=0; i<model->num_targets; i++) {
            if (index != i) {
                double distance = double_matrix_get_zero(model->distances, index, i);
                if (distance > 0) {
                    unsigned int category = unsigned_array_get(model->assignments, i);
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
            unsigned int new_class;
            switch (model->update_method) {
                case NEAREST:
                    new_class = unsigned_array_get(model->assignments, best_index);
                    unsigned_array_set(model->assignments, index, new_class);
                    break;
                case TOP:
                    unsigned_array_set(model->assignments, index, double_hash_largest_key(class_distances));
                    break;
                case VOTE:
                    best_key = double_hash_largest_key(class_distances);
                    best_value = double_hash_get(class_distances, best_key)->value;
                    if (best_value > model->threshold) {
                        unsigned_array_set(model->assignments, index, best_key);
                    }
                    break;
                default:
                    printf("Update method not recognized, freaking out.\n");
                    exit(1);
                    break;
            }
        }
        double_hash_free(class_distances);
    }
    free(target_indices);
}

void CW_train(CW *model)
{
    srand(time(NULL));
    
    static_cw_model = model;
    static_progress = progressbar_new("Building Representations", model->corpus->document_count);
    target_corpus_each_document(model->corpus, &CW_each_document);
    progressbar_finish(static_progress);
    
    int iteration_count = 50;
    progressbar *iteration_progress = progressbar_new("Iterating", iteration_count);
    for (int i=0; i<iteration_count; i++) {
        CW_iteration(model);
        progressbar_inc(iteration_progress);
    }
    progressbar_finish(iteration_progress);
    
    static_cw_model = NULL;
    CW_save_target_wordmap(model);
}

const char *CW_method_str_map(CW *model)
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

void CW_save_wordmap(CW *model)
{
    char *wordmap_f = (char *)malloc(strlen(model->corpus_filename)+9);
    sprintf(wordmap_f,"%s.wordmap",model->corpus_filename);
    WordMap_dump(model->corpus->wordmap, wordmap_f);
    
    free(wordmap_f);
}

void CW_save_categorization(CW *model)
{
    char save_f[60];
    sprintf(save_f,"%s.%d.%s.focw",model->corpus_filename,model->document_index,CW_method_str_map(model));
    printf("Saving categorization %s\n",save_f);
    FILE *fout = fopen(save_f,"w");
    for (int i=0; i<model->num_targets; i++) {
        int index = hash_reverse_lookup(model->wordmap_to_target, i)->key;
        fprintf(fout,"%d\t%d\n",index,unsigned_array_get(model->assignments, i));
    }
    fclose(fout);
}

FILE *static_save_file;

void CW_save_target_wordmap(CW *model)
{
    char save_f[60];
    sprintf(save_f,"%s.%d.%s.wmt",model->corpus_filename,model->document_index,CW_method_str_map(model));
    printf("Saving wordmap %s\n",save_f);
    hash_save(model->wordmap_to_target, save_f);
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
unsigned int CW_static_f_ix;
void CW_ppmi_helper(hash_element *element)
{
    // element = {key => context_word, value => f_ij}
    unsigned int f_ij = element->value;
    
    hash_element *context_element = hash_get(static_cw_model->context_counts, element->key);
    unsigned int f_xj = context_element->value;
    
    unsigned int f_xx = static_cw_model->f_xx;
    unsigned int f_ix = CW_static_f_ix;
    
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

double_hash *CW_ppmi(CW *model,int index)
{
    double_hash *target_ppmi = double_hash_new(10);
    CW_static_f_ix = model->targets[index]->sum;
    
    static_ppmi = target_ppmi;
    hash_foreach(model->targets[index], &CW_ppmi_helper);
    
    return target_ppmi;
}
