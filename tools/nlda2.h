#ifndef NLDA_2_H
#define NLDA_2_H

#include "document_corpus.h"
#include "SparseCounts.h"
#include "count_list.h"
#include "Instance.h"
#include "progressbar.h"

typedef struct nLDA_t
{
	document_corpus *corpus;
	unsigned int categories;
	Instance *instances;
	unsigned int document_index;
	count_list *ncds;
	count_list *nwcs;
	double alpha,beta,gamma;
    char *corpus_filename;
    char *output_dir;
    
    ct_hash *unique_words;
    unsigned int word_count;
} nLDA;

nLDA *nLDA_new(double alpha, double beta, double gamma, char *filename, char *output_dir);
void nLDA_train(nLDA *model);
void nLDA_free(nLDA *model);
void nLDA_assign_category(nLDA *model, Instance *instance);
double nLDA_P_w_cnew(nLDA *model, Instance *instance);
double nLDA_P_cnew_d(nLDA *model, Instance *instance);
double nLDA_P_c_d(nLDA *model, Instance *instance, unsigned int category);
double nLDA_P_w_c(nLDA *model, Instance *instance, unsigned int category);
void nLDA_dump(nLDA *model);
void nLDA_save_representations(nLDA *model, char *filename);
void nLDA_reassess(nLDA *model);

#endif

