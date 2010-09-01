#include "focw.h"
#include "LSH.h"

int main(int argc, const char **argv)
{
	if(argc != 2) {
		printf("focw expects 1 argument, recieved %d\n",argc-1);
		printf("\nUsage: focw <target_corpus>\n");
		return 1;
	}
    
    srand(time(0));
    FoCW *model = FoCW_new(argv[1]);
    FoCW_train(model);
    FoCW_free(model);
    
	return 0;
}


FoCW *FoCW_new(const char *filename)
{
    FoCW *model = (FoCW *)malloc(sizeof(FoCW));
    model->corpus = target_corpus_new((char *)filename);
    model->corpus_filename = filename;
    
    model->num_targets = 16;
    model->targets = (unsigned_array **)malloc(sizeof(unsigned_array *)*model->num_targets);
    model->categories = (int *)malloc(sizeof(int)*model->num_targets);
    for (int i=0; i<model->num_targets; i++) {
        model->targets[i] = unsigned_array_new(32);
        model->categories[i] = -1;
    }
    model->next_category = 0;
    
    LSH_setup();
    
    return model;
}

FoCW *static_model;
progressbar *static_progress;
int document_index;
void FoCW_train(FoCW *model)
{
    static_model = model;
    document_index = 0;
    static_progress = progressbar_new("Training", model->corpus->document_count);
    target_corpus_each_document(model->corpus, &FoCW_each_document);
    progressbar_finish(static_progress);
    static_model = NULL;
}

void FoCW_each_document(unsigned int target, unsigned int *words, unsigned int length)
{
    // Update representation after each document
    unsigned_array *rep = FoCW_get_target(static_model, target);
    for (int i=0; i<length; i++) {
        unsigned_array_add(rep, words[i], 1);
    }
    
    // Is this a new item?
    if (static_model->categories[target] == -1) {
        // ...then set it to a new category
        static_model->categories[target] = static_model->next_category++;
    }
    
    // Update category assignments
        // Iterate through elements in random order
    unsigned int *order = (unsigned int *)malloc(sizeof(unsigned int)*static_model->num_targets);
    for (int i=0; i<static_model->num_targets; i++) {
        order[i] = i;
    }
    FoCW_shuffle(order, static_model->num_targets);
        // Get d symmetric random unit vectors
    unsigned int k = static_model->corpus->wordmap->size;
    for (int i=0; i<PRECISION; i++) {
        static_model->unit_vectors[i] = LSH_gen_unit_vector(k);
    }
    for (int j=0; j<static_model->num_targets; j++) {
        target = order[j];
        rep = FoCW_get_target(static_model, target);
        unsigned int closest = 0;
        unsigned int distance = static_model->corpus->wordmap->size;
        unsigned int target_signature = LSH_signature(rep, static_model->unit_vectors, k);
        for (int i=0; i<static_model->num_targets; i++) {
            if(i != target) {
                unsigned_array *item = static_model->targets[order[i]];
                unsigned int item_signature = LSH_signature(item, static_model->unit_vectors, k);
                unsigned int test_distance = LSH_distance(target_signature, item_signature);
                if (test_distance < distance) {
                    distance = test_distance;
                    closest = i;
                }
            }
        }
        
        // `closest' is the index of the closest item; assign its category to `target'
        printf("%f\n",distance);
        static_model->categories[target] = static_model->categories[closest];
    }
    
    progressbar_inc(static_progress);
}

unsigned_array *FoCW_get_target(FoCW *model, unsigned int index)
{
    unsigned_array *target = NULL;
    if (index >= model->num_targets) { // We asked for a target that hasn't been allocated or initialized...
        int new_max = model->num_targets*2;
        while (index >= new_max) {
            new_max *= 2;
        }
        unsigned_array **new_targets = (unsigned_array **)malloc(sizeof(unsigned_array *) * new_max);
        for (int i=0; i<model->num_targets; i++) {
            new_targets[i] = model->targets[i];
        }
        for (int i=model->num_targets; i<new_max; i++) {
            new_targets[i] = unsigned_array_new(32);
        }
        model->num_targets = new_max;
        free(model->targets);
        model->targets = new_targets;
    }
    target = model->targets[index];
    return target;
}

void FoCW_free(FoCW *model)
{
    target_corpus_free(model->corpus);
    LSH_teardown();
    free(model);
}

// Helper function for shuffling an array.
// http://benpfaff.org/writings/clc/shuffle.html
void FoCW_shuffle(unsigned int *array, unsigned int n)
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