#ifndef NLDA_H
#define NLDA_H

#include "Instance.h"
#include "context_corpus.h"
#include "count_list.h"
#include "WordMap.h"
#include <stdlib.h>
#include <time.h>
#include "progressbar.h"

#define NLDA_WORDMAP_BUCKETS 400

typedef struct t_nLDA
{
	count_list *nwc;
	count_list *ncd;
	Instance *instances;
	Instance *last_instance;
	unsigned int instance_count;
	unsigned int categories;
	double alpha;
	double beta;
	double gamma;
	unsigned int empty_category;
	context_corpus *corpus;
	WordMap *wordmap;
	unsigned int word_count;
	unsigned int current_document;
	char *prefix;
} nLDA;

nLDA *nLDA_new(double alpha, double beta,char *prefix);
void nLDA_gibbs(nLDA *nlda, context_corpus *corpus, unsigned int iterations);
void nLDA_dump(nLDA *nlda, char *file);
void nLDA_free(nLDA *nlda);
SparseCounts *nLDA_get_nwc(nLDA *nlda, unsigned int category);
SparseCounts *nLDA_get_ncd(nLDA *nlda, unsigned int document);
void nLDA_gibbs_init(nLDA *nlda);
void nLDA_gibbs_iteration(nLDA *nlda, unsigned int iteration);

unsigned int nLDA_sample_category(nLDA *nlda, unsigned int word);
unsigned int nLDA_new_category(nLDA *nlda);
double nLDA_P_c(nLDA *nlda, unsigned int category);
double nLDA_P_w_c(nLDA *nlda, unsigned int category, unsigned int word);
double nLDA_P_c_new(nLDA *nlda);
double nLDA_P_w_c_new(nLDA *nlda, unsigned int word);

#endif
