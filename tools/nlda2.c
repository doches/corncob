#include "nlda2.h"
#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if(argc < 6) {
		printf("nlda2 expects 5 arguments, received %d\n",argc-1);
		printf("\nUsage: nlda2 [alpha] [beta] [gamma] [corpusfile] [output dir]\n");
		return 1;
	}
	double alpha = atof(argv[1]);
	double beta = atof(argv[2]);
	double gamma = atof(argv[3]);
	srandom(time(NULL));
	nLDA *model = nLDA_new(alpha,beta,gamma,argv[4],argv[5]);
	nLDA_train(model);
    nLDA_dump(model);
	return 0;
}

nLDA *nLDA_new(double alpha, double beta, double gamma, char *filename, char *output_dir)
{
	nLDA *new = (nLDA *)malloc(sizeof(nLDA));
  if (filename[strlen(filename)-1] == '/') {
    filename[strlen(filename)-1] = '\0';
  }
	new->corpus = document_corpus_new(filename);
  new->corpus_filename = filename;
  printf("[%s]\n",new->corpus_filename);
  if (output_dir[strlen(output_dir)-1] == '/') {
    output_dir[strlen(output_dir)-1] = '\0';
  }
  new->output_dir = output_dir;
	new->categories = 0;
	new->instances = NULL;
	new->document_index = 0;
	new->alpha = alpha;
	new->beta = beta;
	new->gamma = gamma;
	new->ncds = count_list_new();
	new->nwcs = count_list_new();
  new->unique_words = hash_new(1000);
  new->word_count = 0;
	return new;
}

nLDA *static_model = NULL; // Use this with caution!
unsigned int static_instance_index = 0; // Take care when touching statics.
Instance *static_instance_last = NULL;
progressbar *progress = NULL;
void nLDA_train_each_document(unsigned int *words, unsigned int length)
{
	count_list_add(static_model->ncds);
  progressbar *document_progress = progressbar_new("Document", length);
	for(int i=0;i<length;i++) {
    // Update list of unique words (W)
    hash_element *unique_element = hash_get(static_model->unique_words, words[i]);
    if (unique_element == NULL) {
      hash_add(static_model->unique_words, words[i], 1);
      static_model->word_count++;
    }
       
    // Assign a category to this word/document instance
		Instance *instance = Instance_new(words[i],0,static_model->document_index,static_instance_index++,NULL);
		nLDA_assign_category(static_model,instance);
		if(static_model->instances == NULL) {
			static_model->instances = instance;
		} else {
			static_instance_last->next = instance;
		}
		static_instance_last = instance;
    progressbar_inc(document_progress);
	}
	static_model->document_index++;
	progressbar_inc(progress);
  progressbar_finish(document_progress);
  if (static_model->document_index % 10 == 0) {
    nLDA_dump(static_model);
  }
}

void nLDA_dump(nLDA *model)
{
  char filename[200];
  filename[0] = '\0';
  sprintf(filename,"%s/%s.%d.%fa.%fb.%fg.nlda",model->output_dir,model->corpus_filename,model->document_index,model->alpha,model->beta,model->gamma);
  printf("Saving categorization %s\n",filename);
    
	Instance *instance = Instance_new(0,0,0,0,NULL);
	FILE *fout = fopen(filename,"w");
	for(int w=0;w<model->corpus->wordmap->size;w++) {
		instance->w_i = w;
		int best = 0;
		double best_p = 0.0f;
		for(int i=0;i<model->categories;i++) {
			double temp_p = nLDA_P_w_c(model,instance,i);
			if(temp_p > best_p) {
				best = i;
				best_p = temp_p;
			}
		}
		fprintf(fout,"%d %d\n",w,best);
	}
	fclose(fout);
	char wfname[200] = "";
	strcpy((char *)wfname,filename);
	strcat((char *)wfname,".wordmap");
	WordMap_dump(model->corpus->wordmap,wfname);
	Instance_free(instance);
}

unsigned int nLDA_new_category(nLDA *model)
{
	count_list_add(model->nwcs);
	return model->categories++;
}

double static_all_z_in_d;
void nLDA_assign_category(nLDA *model, Instance *instance)
{
	if(model->categories == 0) {
        // If this is the first word (e.g. if we don't yet have _any_ categories) 
        // create a new category, assign it to this instance, and return.
		instance->z_i = nLDA_new_category(model);
		SparseCounts_add(count_list_get(model->nwcs,instance->z_i),instance->w_i,1);
		SparseCounts_add(count_list_get(model->ncds,instance->d_i),instance->z_i,1);
		return;
	}
	
  // Compute the probability of each existing category given this word
  static_all_z_in_d = 0.0f;
  SparseCounts *ncd = count_list_get(model->ncds, instance->d_i);
  for (int cat_iter = 0; cat_iter < model->categories; cat_iter++) {
    static_all_z_in_d += (SparseCounts_getValue(ncd, cat_iter) + model->alpha + model->word_count * model->beta) * (1-model->gamma);
  }
  static_all_z_in_d += (model->alpha + model->word_count * model->beta) * model->gamma;
    
	double *probabilities = (double *)malloc(sizeof(double)*(model->categories+1));
	double total = 0.0;
	for(unsigned int i=0;i<model->categories;i++) {
		probabilities[i] = nLDA_P_w_c(model,instance,i) * nLDA_P_c_d(model,instance,i);
		total += probabilities[i];
	}
    // Include the probability that this word came from a new category
	probabilities[model->categories] = nLDA_P_w_cnew(model,instance) * nLDA_P_cnew_d(model,instance);
	total += probabilities[model->categories];
    
    // Sample a random category based on the just-computed probabilities
	double sample = ((random()%1000)/(1000.0)) * total;
	for(unsigned int i=0;i<model->categories;i++) {
		sample -= probabilities[i];
		if(sample <= 0.0f) {
			free(probabilities);
			instance->z_i = i;
			SparseCounts_add(count_list_get(model->nwcs,instance->z_i),instance->w_i,1);
			SparseCounts_add(count_list_get(model->ncds,instance->d_i),instance->z_i,1);
            
			return;
		}
	}
    
  // If we got here, we should create a new category...
	instance->z_i = nLDA_new_category(model);
	SparseCounts_add(count_list_get(model->nwcs,instance->z_i),instance->w_i,1);
	SparseCounts_add(count_list_get(model->ncds,instance->d_i),instance->z_i,1);
    
  free(probabilities);
}

double nLDA_P_w_c(nLDA *model, Instance *instance, unsigned int category)
{
    SparseCounts *nwc = count_list_get(model->nwcs, category);
    double numerator = SparseCounts_getValue(nwc, instance->w_i) + model->beta;
    double denominator = (nwc->total + model->word_count * model->beta);
    
    return numerator / denominator;
}

double nLDA_P_c_d(nLDA *model, Instance *instance, unsigned int category)
{
    SparseCounts *ncd = count_list_get(model->ncds, instance->d_i);
    double numerator = (SparseCounts_getValue(ncd, category) + model->alpha + model->word_count * model->beta) * (1-model->gamma);
    double denominator = static_all_z_in_d;
    
    return numerator / denominator;
}

double nLDA_P_w_cnew(nLDA *model, Instance *instance)
{
    double numerator = model->beta;
    double denominator = model->word_count * model->beta;
    
    return numerator / denominator;
}

double nLDA_P_cnew_d(nLDA *model, Instance *instance)
{
    double numerator = (model->alpha + model->word_count * model->beta) * model->gamma;
    double denominator = static_all_z_in_d;
    
    return numerator / denominator;
}

void nLDA_train(nLDA *model)
{
	static_model = model;
	static_instance_index = 0;
	
	progress = progressbar_new("Training",model->corpus->document_count);
	document_corpus_each_document(model->corpus, &nLDA_train_each_document);
	progressbar_finish(progress);
	
	static_instance_index = -1;
	static_model = NULL;
}

void nLDA_free(nLDA *model)
{
	Instance_free(model->instances);
	free(model);
}
