#include "rmc.h"
#include "corpus.h"

RMC *rmc;
void RMC_each_word(char *word)
{
	unsigned w_i = WordMap_index(rmc->wordmap,word);
	printf("%d:%d\n",w_i,RMC_sample_category(rmc,w_i));
}

void RMC_each_document(char *filename)
{
	document_each_word(filename, &RMC_each_word);
	// TODO increment $d$
}

int main(void)
{
	rmc = RMC_new(0.5,5.0);
	corpus *data = corpus_new("toy", "xml", 10);
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

RMC *RMC_new(double alpha, double beta)
{
	srand(time(0));

	RMC *new = malloc(sizeof(RMC));
	new->alpha = alpha;
	new->beta = beta;
	new->categories = 0;
	new->n_w_c = NULL;
	new->wordmap = WordMap_new(WORDMAP_BUCKETS);
	new->instances = NULL;
	return new;
}

void RMC_free(RMC *rmc)
{
	count_list_free(rmc->n_w_c);
	WordMap_free(rmc->wordmap);
	Instance_free(rmc->instances);
	free(rmc);
}

double RMC_P_c_new(RMC *rmc)
{
	double numerator = (1.0 - rmc->beta);
	double denominator = (1.0 - rmc->beta) + rmc->categories * rmc->beta;
	return numerator / denominator;
}

SparseCounts *RMC_get_n_w_c(RMC *rmc, unsigned int category)
{
	int current = 0;
	count_list *counts = rmc->n_w_c;
	while(counts != NULL && current < category) {
		counts = counts->next;
		current++;
	}
	if(counts != NULL) {
		return counts->counts;
	} else {
		return NULL;
	}
}

double RMC_P_c(RMC *rmc, unsigned int category)
{
	SparseCounts *n_w_c = RMC_get_n_w_c(rmc,category);
	double numerator = n_w_c->total * rmc->beta;
	double denominator = (1.0 - rmc->beta) + rmc->categories * rmc->beta;
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
	}
	probs[rmc->categories] = RMC_P_c_new(rmc);
	total += probs[rmc->categories];
	double sample = (rand()/(double)RAND_MAX) * total;
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
