#ifndef RMC_H
#define RMC_H

#include "SparseCounts.h"
#include "WordMap.h"
#include "Instance.h"
#include <stdlib.h>
#include <time.h>

#define COUNT_LIST_BUCKETS 100
#define WORDMAP_BUCKETS 50

typedef struct t_count_list
{
	SparseCounts *counts;
	struct t_count_list *next;
} count_list;

count_list *count_list_new();
void count_list_free(count_list *);

typedef struct t_RMC
{
	count_list *n_w_c;
	WordMap *wordmap;
	Instance *instances;
	unsigned int categories;
	double alpha;
	double beta;
} RMC;

RMC *RMC_new(double alpha, double beta);
void RMC_free(RMC *rmc);
double RMC_P_c_w(RMC *rmc, unsigned int category, unsigned int word);
double RMC_P_c(RMC *rmc, unsigned int category);
double RMC_P_c_new(RMC *rmc);
double RMC_P_w_c(RMC *rmc, unsigned int word, unsigned int category);
unsigned int RMC_sample_category(RMC *rmc, unsigned int word);
SparseCounts *RMC_get_n_w_c(RMC *rmc, unsigned int category);
unsigned int RMC_new_category(RMC *rmc);

#endif
