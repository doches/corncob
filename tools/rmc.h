#ifndef RMC_H
#define RMC_H

#include "SparseCounts.h"
#include "WordMap.h"
#include "Instance.h"
#include "count_list.h"
#include <stdlib.h>
#include <time.h>

#define WORDMAP_BUCKETS 1000

typedef struct t_RMC
{
	count_list *n_w_c;
	count_list *n_c_d;
	WordMap *wordmap;
	Instance *instances;
	Instance *last_instance;
	unsigned int categories;
	double alpha;
	double beta;
	unsigned int empty_category;
} RMC;

RMC *RMC_new(double alpha, double beta);
void RMC_free(RMC *rmc);
double RMC_P_c(RMC *rmc, unsigned int category);
double RMC_P_c_new(RMC *rmc);
double RMC_P_w_c(RMC *rmc, unsigned int word, unsigned int category);
double RMC_P_w_c_new(RMC *rmc, unsigned int word);
unsigned int RMC_sample_category(RMC *rmc, unsigned int word);
SparseCounts *RMC_get_n_w_c(RMC *rmc, unsigned int category);
SparseCounts *RMC_get_n_c_d(RMC *rmc, unsigned int document);
unsigned int RMC_new_category(RMC *rmc);
void RMC_add_instance(Instance *i);
void RMC_gibbs(RMC *rmc,unsigned int iterations);
unsigned int RMC_gibbs_iteration(RMC *rmc);
void RMC_dump(RMC *rmc, char *prefix);

#endif
