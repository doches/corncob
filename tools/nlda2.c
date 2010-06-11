#include "nlda2.h"

unsigned int reassess_window = 0;
int main(int argc, char **argv)
{
	if(argc < 6) {
		printf("nlda2 expects 5-6 arguments, received %d\n",argc-1);
		printf("\nUsage: nlda2 [alpha] [beta] [gamma] [corpusfile] [outfile] <reassess_window>\n");
		return 1;
	}
	double alpha = atof(argv[1]);
	double beta = atof(argv[2]);
	double gamma = atof(argv[3]);
	reassess_window = 10;
	if(argc == 7) {
		reassess_window = atoi(argv[6]);
	}
	nLDA *model = nLDA_new(alpha,beta,gamma,argv[4]);
	nLDA_train(model);
	nLDA_dump(model,argv[5]);
	nLDA_free(model);
	return 0;
}

nLDA *nLDA_new(double alpha, double beta, double gamma, char *filename)
{
	nLDA *new = (nLDA *)malloc(sizeof(nLDA));
	new->corpus = line_corpus_new(filename);
	new->categories = 0;
	new->instances = NULL;
	new->document_index = 0;
	new->alpha = alpha;
	new->beta = beta;
	new->gamma = gamma;
	new->ncds = count_list_new();
	new->nwcs = count_list_new();
	return new;
}

nLDA *static_model = NULL; // Use this with caution!
unsigned int static_instance_index = 0; // Take care when touching statics.
Instance *static_instance_last = NULL;
progressbar *progress = NULL;
void nLDA_train_each_document(unsigned int *words, unsigned int length)
{
	count_list_add(static_model->ncds);
	for(int i=0;i<length;i++) {
		nLDA_reassess(static_model);
		
		Instance *instance = Instance_new(words[i],0,static_model->document_index,static_instance_index++,NULL);
		nLDA_assign_category(static_model,instance);
		if(static_model->instances == NULL) {
			static_model->instances = instance;
		} else {
			static_instance_last->next = instance;
		}
		static_instance_last = instance;
	}
	static_model->document_index++;
	progressbar_inc(progress);
}

void nLDA_dump(nLDA *model, char *filename)
{
	Instance *instance = Instance_new(0,0,0,0,NULL);
	FILE *fout = fopen(filename,"w");
	for(int w=0;w<model->corpus->wordmap->size;w++) {
		instance->w_i = w;
		unsigned int best = 0;
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
	WordMap_dump(model->corpus->wordmap,"nlda2_wordmap");
	Instance_free(instance);
}

unsigned int nLDA_new_category(nLDA *model)
{
	count_list_add(model->nwcs);
	return model->categories++;
}

unsigned int reassess_count = 0;
Instance *reassess_start = NULL;
void nLDA_reassess(nLDA *model)
{
	Instance *instance = model->instances;
	if (reassess_start != NULL) {
		instance = reassess_start;
		reassess_start = reassess_start->next;
	}
	reassess_count = 0;
	while(instance != NULL) {
		SparseCounts *ncd = count_list_get(model->ncds,instance->d_i);
		SparseCounts *nwc = count_list_get(model->nwcs,instance->z_i);
		SparseCounts_add(ncd,instance->z_i,-1);
		SparseCounts_add(nwc,instance->w_i,-1);
		model->document_index = instance->d_i;
		unsigned int old_z = instance->z_i;
		nLDA_assign_category(model,instance);
		if(old_z != instance->z_i) {
			nwc = count_list_get(model->nwcs,instance->z_i);
		}
		SparseCounts_add(ncd,instance->z_i,1);
		SparseCounts_add(nwc,instance->w_i,1);
	
		instance = instance->next;
		reassess_count++;
	}

	if(reassess_count >= reassess_window && reassess_start == NULL) {
		reassess_start = model->instances;
	}
}

void nLDA_assign_category(nLDA *model, Instance *instance)
{
	if(model->categories == 0) {
		instance->z_i = nLDA_new_category(model);
		SparseCounts_add(count_list_get(model->nwcs,instance->z_i),instance->w_i,1);
		SparseCounts_add(count_list_get(model->ncds,instance->d_i),instance->z_i,1);
		return;
	}
	
	double *probabilities = (double *)malloc(sizeof(double)*(model->categories+1));
	double total = 0.0;
	for(unsigned int i=0;i<model->categories;i++) {
		probabilities[i] = nLDA_P_w_c(model,instance,i) * nLDA_P_c(model,i);
		total += probabilities[i];
	}
	probabilities[model->categories] = nLDA_P_w_c_new(model,instance) * nLDA_P_c_new(model);
	total += probabilities[model->categories];
	double sample = ((arc4random()%1000)/(1000.0)) * total;
	
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
	instance->z_i = nLDA_new_category(model);
	SparseCounts_add(count_list_get(model->nwcs,instance->z_i),instance->w_i,1);
	SparseCounts_add(count_list_get(model->ncds,instance->d_i),instance->z_i,1);
}

double nLDA_P_w_c(nLDA *model, Instance *instance, unsigned int c)
{
	SparseCounts *nwc = count_list_get(model->nwcs,c);
	double numerator = (SparseCounts_getValue(nwc,instance->w_i) + model->alpha);
	double denominator = (nwc->total + model->corpus->wordmap->size * model->alpha);	
	return numerator / denominator;
}

double nLDA_P_c(nLDA *model, unsigned int category)
{
	SparseCounts *ncd = count_list_get(model->ncds,model->document_index);
	while(ncd == NULL) {
		count_list_add(model->ncds);
		ncd = count_list_get(model->ncds,category);
	}
	double numerator = ((SparseCounts_getValue(ncd,category)+model->gamma)*model->beta);
	double denominator = ((1.0 - model->beta) + (ncd->total + model->categories*model->gamma) * model->beta);
	return numerator / denominator;
}

double nLDA_P_w_c_new(nLDA *model, Instance *instance)
{
	double numerator = (model->alpha);
	double denominator = (model->corpus->wordmap->size * model->alpha);
	return numerator / denominator;
}

double nLDA_P_c_new(nLDA *model)
{
	SparseCounts *ncd = count_list_get(model->ncds,model->document_index);
	while(ncd == NULL) {
		count_list_add(model->ncds);
		ncd = count_list_get(model->ncds,model->document_index);
	}
	double numerator = (1.0 - model->beta);
	double denominator = ((1.0 - model->beta) + (ncd->total + model->categories * model->gamma) * model->beta);
	return numerator / denominator;
}

void nLDA_train(nLDA *model)
{
	static_model = model;
	static_instance_index = 0;
	
	progress = progressbar_new("Training",model->corpus->document_count);
	line_corpus_each_document(model->corpus, &nLDA_train_each_document);
	progressbar_finish(progress);
	
	static_instance_index = -1;
	static_model = NULL;
}

void nLDA_free(nLDA *model)
{
	line_corpus_free(model->corpus);
	Instance_free(model->instances);
	free(model);
}