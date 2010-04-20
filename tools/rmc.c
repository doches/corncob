#include "rmc.h"
#include "corpus.h"
#include <stdio.h>

#define ALPHA 0.2f
#define BETA 0.9f

RMC *rmc;
unsigned int current_document = 0;

void RMC_each_word(char *word)
{
	unsigned w_i = WordMap_index(rmc->wordmap,word);
	
	// Add instance
	unsigned int z = 0;
	z = RMC_sample_category(rmc,w_i);
	Instance *i = Instance_new(w_i, z, current_document, -1, word);
	if(rmc->instances == NULL) {
		rmc->instances = i;
		rmc->last_instance = i;
		i->index = 0;
	} else {
		i->index = rmc->last_instance->index+1;
		rmc->last_instance->next = i;
		rmc->last_instance = i;
	}
//	Instance_print(i);
	if(i->index % 10 == 0) { printf("."); fflush(stdout); }
	
	// Update counts
	SparseCounts *nwc = RMC_get_n_w_c(rmc, i->z_i);
	SparseCounts_add(nwc,w_i,1);
	SparseCounts *ncd = RMC_get_n_c_d(rmc, current_document);
	SparseCounts_add(ncd,z,1);
}

void RMC_each_document(char *filename)
{
	printf("%s...\n",filename);
	if(rmc->n_c_d == NULL) {
		rmc->n_c_d = count_list_new();
	} else {
		count_list *end = rmc->n_c_d;
		while(end->next != NULL) { end = end->next; }
		count_list *new = count_list_new();
		end->next = new;
	}
//	RMC_sample_category(rmc,0);
	document_each_word(filename, &RMC_each_word);
	SparseCounts *ncd = RMC_get_n_c_d(rmc, current_document);
	printf("\n<%d",SparseCounts_getValue(ncd,0));
	for(int k=1;k<rmc->categories;k++) {
		printf(", %d",SparseCounts_getValue(ncd,k));
	}
	printf(">\n");
	current_document++;
}

int main(void)
{
	rmc = RMC_new(ALPHA,BETA);
//	corpus *data = corpus_new("toy", "xml", 10);
	corpus *data = corpus_new("bnc_small", "xml", 10);
	corpus_each_document(data, &RMC_each_document);
}

/******** Implementation **************/

count_list *count_list_new()
{
	count_list *new = malloc(sizeof(count_list));
	new->counts = SparseCounts_new(COUNT_LIST_BUCKETS);
	new->next = NULL;
	return new;
}

void count_list_free(count_list *list)
{
	count_list *next = NULL;
	while(list != NULL) {
		next = list->next;
		SparseCounts_free(list->counts);
		free(list);
		list = next;
	}
}

SparseCounts *count_list_get(count_list *start, unsigned int max)
{
	int current = 0;
	count_list *counts = start;
	while(counts != NULL && current < max) {
		counts = counts->next;
		current++;
	}
	if(counts != NULL) {
		return counts->counts;
	} else {
		return NULL;
	}
}

RMC *RMC_new(double alpha, double beta)
{
	srand(time(0));

	RMC *new = malloc(sizeof(RMC));
	new->alpha = alpha;
	new->beta = beta;
	new->categories = 0;
	new->n_w_c = NULL;
	new->n_c_d = NULL;
	new->wordmap = WordMap_new(WORDMAP_BUCKETS);
	new->instances = NULL;
	new->last_instance = NULL;
	return new;
}

void RMC_free(RMC *rmc)
{
	count_list_free(rmc->n_w_c);
	WordMap_free(rmc->wordmap);
	Instance_free(rmc->instances);
	free(rmc);
}

SparseCounts *RMC_last_nwc = NULL;
unsigned int RMC_last_nwc_cat = -1;
SparseCounts *RMC_get_n_w_c(RMC *rmc, unsigned int category)
{
	if(RMC_last_nwc_cat != category) {
		RMC_last_nwc = count_list_get(rmc->n_w_c,category);
		RMC_last_nwc_cat = category;
	}
	return RMC_last_nwc;
}

SparseCounts *RMC_last_ncd = NULL;
unsigned int RMC_last_ncd_doc = -1;
SparseCounts *RMC_get_n_c_d(RMC *rmc, unsigned int document)
{
	if(RMC_last_ncd_doc != document) {
		RMC_last_ncd = count_list_get(rmc->n_c_d,document);
		RMC_last_ncd_doc = document;	
	}
	
	return RMC_last_ncd;	
}

#define GAMMA 0.1f

double RMC_P_c_new(RMC *rmc)
{
	SparseCounts *ncd = RMC_get_n_c_d(rmc,current_document);
	double numerator = (1.0 - rmc->beta);
	double denominator = (1.0 - rmc->beta) + (ncd->total+rmc->categories*GAMMA) * rmc->beta;
	return numerator / denominator;
}

double RMC_P_c(RMC *rmc, unsigned int category)
{
	SparseCounts *ncd = RMC_get_n_c_d(rmc,current_document);
	double numerator = (SparseCounts_getValue(ncd,category)+GAMMA) * rmc->beta;
	double denominator = (1.0 - rmc->beta) + (ncd->total+rmc->categories*GAMMA) * rmc->beta;
	return numerator / denominator;
}

double RMC_P_w_c(RMC *rmc, unsigned int word, unsigned int category)
{
	SparseCounts *n_w_c = RMC_get_n_w_c(rmc,category);
	
	double numerator = SparseCounts_getValue(n_w_c, word) + rmc->alpha;
	double denominator = n_w_c->total + rmc->wordmap->size * rmc->alpha;
	
	return numerator / denominator;
}

double RMC_P_c_w(RMC *rmc, unsigned int category, unsigned int word)
{
	double numerator = RMC_P_w_c(rmc, word, category) * RMC_P_c(rmc, category);
	double denominator = 0;
	for(int k=0;k < rmc->categories; k++)
	{
		denominator += RMC_P_w_c(rmc, word, k) * RMC_P_c(rmc,k);
	}
	return numerator / denominator;
}

unsigned int RMC_new_category(RMC *rmc)
{
	if(rmc->n_w_c == NULL) {
		rmc->n_w_c = count_list_new();
	} else {
		count_list *list = rmc->n_w_c;
		while(list->next != NULL) { list = list->next; }
		count_list *new = count_list_new();
		list->next = new;
	}
	return rmc->categories++;
}

unsigned int RMC_sample_category(RMC *rmc, unsigned int word)
{
	if(rmc->categories == 0) {
		return RMC_new_category(rmc);
	}
	
	double *probs = malloc(sizeof(double)*(rmc->categories+1));
	double total = 0.0f;
	for(int k=0;k<rmc->categories;k++)
	{
		probs[k] = RMC_P_c_w(rmc,k,word);
		total += probs[k];
	}
	probs[rmc->categories] = RMC_P_c_new(rmc);
	total += probs[rmc->categories];
	double sample = (rand()/(double)RAND_MAX) * total;
	
	// DEBUG
	/*
	printf("<");
	for(int k=0;k<rmc->categories;k++) { printf("%f,",probs[k]); }
	printf("%f> [%f / %f]\n",probs[rmc->categories],sample,total);
	*/	
	// ENDDEBUG
	
	for(int k=0;k<rmc->categories;k++)
	{
		sample -= probs[k];
		if(sample <= 0.0f) {
			free(probs);
			return k;
		}
	}
	free(probs);
	return RMC_new_category(rmc);
}
