#ifndef NLDA_2_H
#define NLDA_2_H

#include "line_corpus.h"
#include "SparseCounts.h"
#include "count_list.h"
#include "Instance.h"
#include "progressbar.h"

typedef struct nLDA_t
{
	line_corpus *corpus;
	unsigned int categories;
	Instance *instances;
	unsigned int document_index;
	count_list *ncds;
	count_list *nwcs;
	double alpha,beta,gamma;
} nLDA;

nLDA *nLDA_new(double alpha, double beta, double gamma, char *filename);
void nLDA_train(nLDA *model);
void nLDA_free(nLDA *model);
void nLDA_assign_category(nLDA *model, Instance *instance);
double nLDA_P_c_new(nLDA *model);
double nLDA_P_w_c_new(nLDA *model, Instance *instance);
double nLDA_P_c(nLDA *model,unsigned int c);
double nLDA_P_w_c(nLDA *model, Instance *instance, unsigned int c);
void nLDA_dump(nLDA *model, char *filename);
void nLDA_reassess(nLDA *model);

#endif