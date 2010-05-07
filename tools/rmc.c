#include "rmc.h"
#include "corpus.h"
#include <stdio.h>

#define ALPHA 0.9f
#define BETA 0.8f

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
	
	// Gibbs
	RMC_gibbs(rmc,100);
	RMC_dump(rmc,"bnc_small");
}

/******** Implementation **************/

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
	new->empty_category = -1;
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

double RMC_P_w_c_new(RMC *rmc, unsigned int word)
{
	double numerator = rmc->alpha;
	double denominator = rmc->wordmap->size * rmc->alpha;
	
	return numerator / denominator;
}

unsigned int RMC_new_category(RMC *rmc)
{
	count_list *list = rmc->n_w_c;
	unsigned int i = 0;
	while(list != NULL) {
		if(list->counts->total == 0) {
//			printf("re-issuing empty category %d\n",i);
			return i;
		}
		i++;
		list = list->next;
	}
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
	
	// precompute P(c) and P(w|c) \forall c
	double *p_c = malloc(sizeof(double)*(rmc->categories+1));
	double *p_wc = malloc(sizeof(double)*(rmc->categories+1));
	double denominator = 0.0f;
	for(int k=0;k<rmc->categories;k++)
	{
		p_c[k] = RMC_P_c(rmc,k);
		p_wc[k] = RMC_P_w_c(rmc,word,k);
		probs[k] = p_c[k] * p_wc[k];
		denominator += probs[k];
	}
	probs[rmc->categories] = RMC_P_c_new(rmc) * RMC_P_w_c_new(rmc,word);
	denominator += probs[rmc->categories];
	for(int k=0;k<rmc->categories+1;k++) {
		probs[k] /= denominator;
		total += probs[k];
	}
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

void RMC_gibbs(RMC *rmc, unsigned int iterations)
{
	unsigned int changes;
	
	for(int i=0;i<iterations;i++)
	{
		changes = RMC_gibbs_iteration(rmc);
		printf("Gibbs iteration %d: %d z_i updates, %d categories\n",i,changes,rmc->categories);
		fflush(stdout);
	}
}

unsigned int RMC_gibbs_iteration(RMC *rmc)
{
	unsigned int changes = 0,new_z;
	Instance *i = rmc->instances;
	
	SparseCounts *nwc;
	while(i != NULL) {
		nwc = RMC_get_n_w_c(rmc,i->z_i);
		SparseCounts_add(nwc,i->w_i,-1);
		SparseCounts_add(RMC_get_n_c_d(rmc,i->d_i),i->z_i,-1);
		current_document = i->d_i;
		if(nwc->total <= 0) {
			rmc->empty_category = i->z_i;
		}

		new_z = RMC_sample_category(rmc,i->w_i);
		if(new_z != i->z_i) {
			changes++;
			SparseCounts *_nwc = RMC_get_n_w_c(rmc,new_z);
//			printf("w_%d: z_%d (%d) -> z_%d (%d)\n",i->index,i->z_i,nwc->total,new_z,_nwc->total);
			nwc = _nwc;
			i->z_i = new_z;
		}
		SparseCounts_add(nwc,i->w_i,1);
		SparseCounts_add(RMC_get_n_c_d(rmc,i->d_i),i->z_i,1);
		
		i = i->next;
	}
	return changes;
}

void RMC_dump(RMC *rmc, char *prefix)
{
	char *wordmap_f = malloc(sizeof(char) * (strlen(prefix)+8));
	strcpy(wordmap_f,prefix);
	strcat(wordmap_f,".wordmap");
	WordMap_dump(rmc->wordmap,wordmap_f);
	free(wordmap_f);
	
	char *count_f = malloc(sizeof(char) * (strlen(prefix)+7));
	strcpy(count_f,prefix);
	strcat(count_f,".counts");
	FILE *fout = fopen(count_f,"w");
	count_list *list = rmc->n_w_c;
	while(list != NULL)
	{
		for(int w=0;w<rmc->wordmap->size;w++)
		{
			fprintf(fout,"%d ",SparseCounts_getValue(list->counts,w));
		}
		fputs("\n",fout);
		list = list->next;
	}
	fclose(fout);
}
