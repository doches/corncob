#include "nlda.h"
#include "context_corpus.h"
#include <stdio.h>
#include <stdlib.h>

/**** Executable ****/

#define GIBBS_ITER 50

int main(int argc, char *argv[])
{
	if(argc != 7) {
		printf("Usage: nlda <alpha> <beta> <gamma> <corpus_filename> <output_prefix> <iterations>\n");
		printf("\t(Expected 6 arguments, got %d!)\n",argc-1);
		return 1;
	}
	double alpha = atof(argv[1]);
	double beta = atof(argv[2]);
	double gamma = atof(argv[3]);
	context_corpus *corpus = context_corpus_new(argv[4]);
	
	nLDA *nlda = nLDA_new(alpha,beta,gamma,argv[5]);
	nLDA_gibbs(nlda, corpus,atoi(argv[6]));
	nLDA_free(nlda);
	
	return 0;
}

/**** Implementation ****/

nLDA *nLDA_new(double alpha, double beta, double gamma, char *prefix)
{
	srand(time(0));

	nLDA *new = malloc(sizeof(nLDA));
	new->nwc = NULL;
	new->ncd = NULL;
	new->instances = NULL;
	new->last_instance = NULL;
	new->instance_count = 0;
	new->categories = 0;
	new->alpha = alpha;
	new->beta = beta;
	new->gamma = gamma;
	new->empty_category = -1;
	new->corpus = NULL;
	new->wordmap = WordMap_new(NLDA_WORDMAP_BUCKETS);
	new->word_count = 0;
	new->current_document = 0;
	new->prefix = prefix;
	return new;
}

void nLDA_free(nLDA *nlda)
{
	count_list_free(nlda->nwc);
	count_list_free(nlda->ncd);
	Instance_free(nlda->instances);
	context_corpus_free(nlda->corpus);
	WordMap_free(nlda->wordmap);
	free(nlda);
}

void nLDA_gibbs(nLDA *nlda, context_corpus *corpus,unsigned int iterations)
{
	nlda->corpus = corpus;
	
	nLDA_gibbs_init(nlda);
	char filename[40];
	for(int i=0;i<iterations;i++) {
		nLDA_gibbs_iteration(nlda,i);
		
		if(nlda->categories > nlda->word_count) {
			nLDA_reset_categories(nlda);
		}
		
		sprintf(filename,"%stest.%d.probabilities",nlda->prefix,i);
		nLDA_dump(nlda,filename);
	}
}

nLDA *static_nlda; // Use with caution!
progressbar *progress;
int zzz = 0;
void nLDA_gibbs_init_each_document(unsigned int *words,unsigned int size)
{
	// Make a NCD count for this document
	if(static_nlda->ncd == NULL) {
		static_nlda->ncd = count_list_new();
	} else {
		count_list_add(static_nlda->ncd);
	}
//	printf("%d\n",static_nlda->categories);
//	char tpl[30];
//	sprintf(tpl,"Document %d",zzz++);
//	progressbar *tp = progressbar_new(tpl,size);
	for(int i=0;i<size;i++) {
		// Make new Instance & sample category
		unsigned int z = nLDA_sample_category(static_nlda,words[i]);
		Instance *inst = Instance_new(words[i],z,static_nlda->current_document,0,NULL);
		static_nlda->instance_count++;
		if(static_nlda->instances == NULL) {
			static_nlda->instances = inst;
			static_nlda->last_instance = inst;
		} else {
			inst->index = static_nlda->last_instance->index+1;
			static_nlda->last_instance->next = inst;
			static_nlda->last_instance = inst;
		}
		
		// Update counts
		SparseCounts_add(nLDA_get_nwc(static_nlda,z),words[i],1);
		SparseCounts_add(nLDA_get_ncd(static_nlda,static_nlda->current_document),z,1);
		
		// Update wordmap. TODO: this may be wrong!
		if(words[i] > static_nlda->word_count) {
			static_nlda->word_count = words[i];
		}
//		progressbar_inc(tp);
	}
//	progressbar_finish(tp);
//	printf("\n");
	
	static_nlda->current_document++;
	progressbar_inc(progress);
}

void nLDA_gibbs_init(nLDA *nlda)
{
	static_nlda = nlda;
	context_corpus_make_documents(nlda->corpus);
	progress = progressbar_new("Gibbs init",nlda->corpus->document_count);
	context_corpus_each_document(nlda->corpus,&nLDA_gibbs_init_each_document);
	progressbar_finish(progress);
}

void nLDA_gibbs_iteration(nLDA *nlda, unsigned int iteration)
{
	unsigned int new_z;
	Instance *i = nlda->instances;
	
	SparseCounts *nwc,*ncd;
	char label[20];
	sprintf(label,"Gibbs %d",iteration);
	progressbar *progress = progressbar_new(label,nlda->instance_count);
	while(i != NULL) {
		nwc = nLDA_get_nwc(nlda,i->z_i);
		SparseCounts_add(nwc,i->w_i,-1);
		ncd = nLDA_get_ncd(nlda,i->d_i);
		SparseCounts_add(ncd,i->z_i,-1);
		nlda->current_document = i->d_i;
	
		new_z = nLDA_sample_category(nlda,i->w_i);
		if(new_z != i->z_i) {
			if(nwc->total <= 0) {
				nlda->empty_category = i->z_i;
			}
			nwc = nLDA_get_nwc(nlda,new_z);
			i->z_i = new_z;
		}
		SparseCounts_add(nwc,i->w_i,1);
		SparseCounts_add(ncd,i->z_i,1);
	
		i = i->next;
		progressbar_inc(progress);
	}
	progressbar_finish(progress);
}

SparseCounts *nLDA_last_nwc = NULL;
int nLDA_last_nwc_category = -1;
SparseCounts *nLDA_get_nwc(nLDA *nlda, unsigned int category)
{
	if(nLDA_last_nwc_category != category) {
		nLDA_last_nwc = count_list_get(nlda->nwc,category);
		nLDA_last_nwc_category = category;
	}
	
	return nLDA_last_nwc;
}

SparseCounts *nLDA_last_ncd = NULL;
int nLDA_last_ncd_document = -1;
SparseCounts *nLDA_get_ncd(nLDA *nlda, unsigned int document)
{
	if(nLDA_last_ncd_document != document) {
		nLDA_last_ncd = count_list_get(static_nlda->ncd,document);
		nLDA_last_ncd_document = document;
	}
	
	return nLDA_last_ncd;
}

unsigned int nLDA_new_category(nLDA *nlda)
{
	if(nlda->empty_category != -1) {
		int empty = nlda->empty_category;
		nlda->empty_category = -1;
		return empty;
	}
	if(nlda->nwc == NULL) {
		nlda->nwc = count_list_new();
	} else {
		count_list_add(nlda->nwc);
	}
//	printf("Creating category %d\n",nlda->categories);
	return nlda->categories++;
}

unsigned int nLDA_sample_category(nLDA *nlda, unsigned int word)
{
	if(nlda->categories == 0) {
		return nLDA_new_category(nlda);
	}
	
	double *probs = malloc(sizeof(double)*(nlda->categories+1));
	double total = 0.0f;
	
	for(int k=0;k<nlda->categories;k++) {
		probs[k] = nLDA_P_c(nlda,k) * nLDA_P_w_c(nlda,word,k);
		total += probs[k];
	}
	
	probs[nlda->categories] = nLDA_P_w_c_new(nlda,word) * nLDA_P_c_new(nlda);
	total += probs[nlda->categories];
	
	double sample = (rand()/(double)RAND_MAX) * total;
	
	for(int k=0;k<nlda->categories;k++) {
		sample -= probs[k];
		if(sample <= 0.0f) {
			free(probs);
			return k;
		}
	}
	free(probs);
	return nLDA_new_category(nlda);
}

double nLDA_P_c(nLDA *nlda, unsigned int category)
{
	SparseCounts *ncd = nLDA_get_ncd(nlda,nlda->current_document);
	double numerator = ((SparseCounts_getValue(ncd,category)+nlda->gamma)*nlda->beta);
	double denominator = ((1.0 - nlda->beta) + (ncd->total + nlda->categories*nlda->gamma) * nlda->beta);
	return numerator / denominator;
}

double nLDA_P_w_c(nLDA *nlda, unsigned int word, unsigned int category)
{
	SparseCounts *nwc = nLDA_get_nwc(nlda,category);
	double numerator = (SparseCounts_getValue(nwc,word) + nlda->alpha);
	double denominator = (nwc->total + nlda->word_count * nlda->alpha);	
	return numerator / denominator;
}

double nLDA_P_c_new(nLDA *nlda)
{
	SparseCounts *ncd = nLDA_get_ncd(nlda,nlda->current_document);
	double numerator = (1.0 - nlda->beta);
	double denominator = ((1.0 - nlda->beta) + (ncd->total + nlda->categories*nlda->gamma) * nlda->beta);
	return numerator / denominator;
}

double nLDA_P_w_c_new(nLDA *nlda, unsigned int word)
{
	if(nlda->word_count <= 0) {
		nlda->word_count = 1;
	}
	double numerator = (nlda->alpha);
	double denominator = (nlda->word_count * nlda->alpha);
	return numerator / denominator;
}

void nLDA_reset_categories(nLDA *nlda)
{
	ct_hash *word_map = hash_new(nlda->word_count);
	Instance *i = nlda->instances;
	double best_prob = 0.0f;
	int best_cat = -1;
	double temp_prob;
	
	ct_hash *new_category_map = hash_new(nlda->categories);
	unsigned int new_categories = 0;
	
	while(i != NULL) {
		hash_element *elem = hash_get(word_map,i->w_i);

		if(elem == NULL) {
			for(int k=0;k<nlda->categories;k++) {
				temp_prob = nLDA_P_w_c(nlda,i->w_i,k);
				if(temp_prob > best_prob) {
					best_prob = temp_prob;
					best_cat = k;
				}
			}
			elem = hash_add(word_map,i->w_i,best_cat);
			hash_element *new_cat = hash_get(new_category_map,best_cat);
			if(new_cat == NULL) {
				new_cat = hash_add(new_category_map,best_cat,new_categories++);
			}
			elem->value = new_cat->value;
		}
		i->z_i = elem->value;
		
		i = i->next;
	}
	nlda->categories = new_categories;
}

void nLDA_dump(nLDA *nlda, char *file)
{
	FILE *fout = fopen(file,"w");
	for(int w=0;w<=nlda->word_count;w++) {
		fprintf(fout,"%d: ",w);
		for(int k=0;k<nlda->categories;k++) {
			fprintf(fout,"%f ",nLDA_P_w_c(nlda,w,k));
		}
		fprintf(fout,"\n");
	}
	fclose(fout);
	
	fprintf(stderr,"%d categories\n",nlda->categories);
}
