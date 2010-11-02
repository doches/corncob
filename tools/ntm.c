/*
 *  ntm.c
 *  corncob
 *
 *  Created by Trevor Fountain on 10/29/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include <stdlib.h>
#include "ntm.h"
#include "progressbar.h"

int main(int argc, char **argv)
{
    nTM *model = nTM_new("corpora/noise_0", 0.1, 0.1, 0.33);
    
    nTM_train(model);
    
    nTM_free(model);
}

nTM *nTM_new(const char *filename, double alpha, double beta, double gamma)
{
    nTM *model = (nTM *)malloc(sizeof(nTM));
    
    model->filename = filename;
    model->corpus = line_corpus_new(filename);
    
    nTM_save_wordmap(model);
    
    model->interval = 200;
    
    model->alpha = alpha;
    model->beta = beta;
    model->gamma = gamma;
    
    model->nzws = count_list_new();
    model->ndzs = count_list_new();
    model->nzs = SparseCounts_new(100);
    
    model->unique_words = hash_new(20000);
    model->assignments = unsigned_array_new(1000);
    
    model->topics = 0;
    model->documents = 0;
    model->words = 0;
    
    return model;
}

void nTM_save_wordmap(nTM *model)
{
	char wfname[200] = "";
	strcpy((char *)wfname,model->filename);
	strcat((char *)wfname,".wordmap");
	WordMap_dump(model->corpus->wordmap,wfname);
}

void nTM_save_assignments(nTM *model)
{
    char save_f[60];
    sprintf(save_f,"%s.%d.ntm",model->filename,model->documents);
    printf("Saving assignments %s\n",save_f);
    FILE *fout = fopen(save_f,"w");
    for (int i=0; i<model->words; i++) {
        fprintf(fout,"%d\t%d\n",i,unsigned_array_get(model->assignments, i));
    }
    fclose(fout);
}

nTM *_static_nTM = NULL;
progressbar *_static_progressbar;
void nTM_each_document(WordIndex *words, unsigned int length)
{
    // Increment the total number of documents seen
    _static_nTM->documents++;
    
    // Create a new SparseCount for N^d_z (count of topics in this document)
    count_list_add(_static_nTM->ndzs);
    
    for (int i=0; i<length; i++) {
        // Sample a topic from P(z|w)
        TopicIndex assigned_topic = nTM_sample_topic(_static_nTM, words[i], _static_nTM->documents-1);
        
        // If we sampled the new topic, create it first
        if (assigned_topic == _static_nTM->topics) {
            nTM_create_topic(_static_nTM);
        }
        
        
        nTM_assign_topic(_static_nTM,words[i],assigned_topic,_static_nTM->documents);
    }
    progressbar_inc(_static_progressbar);
    if (_static_nTM->documents % _static_nTM->interval == 0) {
        progressbar_finish(_static_progressbar);
        
        nTM_save_assignments(_static_nTM);
        
        _static_progressbar = progressbar_new("Training", _static_nTM->interval);
    }
}

void nTM_assign_topic(nTM *model, WordIndex word, TopicIndex topic, DocumentIndex document)
{
    // Update our assignment for this word
    unsigned_array_set(model->assignments, word, topic);
    
    // Increment the count of how many times this topic has occurred in this this document
    SparseCounts *ndz = count_list_get(model->ndzs, document);
    SparseCounts_add(ndz, topic, 1);
    
    // Increment the count of how many times this word has occurred in this topic
    SparseCounts *nzw = count_list_get(model->nzws, topic);
    SparseCounts_add(nzw, word, 1);
    
    // Increment the count of how many times this topic has occurred overal
    SparseCounts_add(model->nzs, topic, 1);
}

void nTM_create_topic(nTM *model)
{
    // Create a new SparseCount to hold the count of words in this topic
    count_list_add(model->nzws);
    
    SparseCounts_add(model->nzs, model->topics, 0);
    model->topics++;
}

TopicIndex nTM_sample_topic(nTM *model, WordIndex word, DocumentIndex document)
{
    double *topics = (double *)malloc(sizeof(double) * (model->topics+1));
//    printf("P( z | %u ): \n",word);
    for (int z = 0; z < model->topics+1; z++) {
        nTM_see_word(model, word);
        
        topics[z] = nTM_P_z_w(model, z, word);
    }
    
    double sample = ((double)rand()) / RAND_MAX;
    for (int z = 0; z < model->topics+1; z++) {
        sample -= topics[z];
        if (sample <= 0.0f) {
            free(topics);
            return z;
        }
    }
    
    return model->topics;
}

void nTM_see_word(nTM *model, WordIndex word)
{
    hash_element *element = hash_get(model->unique_words, word);
    if (element == NULL) {
        // This is a new word!
        hash_add(model->unique_words, word, 1);
        model->words++;
    }
}

void nTM_train(nTM *model)
{
    _static_nTM = model;
    _static_progressbar = progressbar_new("Training", model->interval);
    count_list_add(model->ndzs);
    line_corpus_each_document(model->corpus, &nTM_each_document);
    progressbar_finish(_static_progressbar);
    _static_nTM = NULL;
}

void nTM_free(nTM *model)
{
    free(model);
}

double nTM_P_d(nTM *model, DocumentIndex document)
{
    unsigned int n_d_all = model->corpus->documents[document].length;
    unsigned int n_z = model->topics;
    double alpha = model->alpha;
    unsigned long n_all = model->words;
    unsigned long n_d = model->documents;
    
    return (n_d_all + n_z * alpha) / 
           (n_all + n_d * n_z * alpha);
}

TopicIndex last_pz_call = 0;
double last_pz_value = -1;
double nTM_P_z(nTM *model, TopicIndex topic)
{
    if (last_pz_value >= 0.0 && last_pz_call == topic) {
        return last_pz_value;
    }
    last_pz_call = topic;
    
    double sum = 0.0;
    
    for (DocumentIndex d = 0; d < model->documents; d++) {
        sum += nTM_P_z_d(model, topic, d) * nTM_P_d(model, d);
    }
    
    last_pz_value = sum;
    
    return sum;
}

double nTM_P_w(nTM *model, WordIndex word)
{
    double sum = 0.0;
    
    for (TopicIndex z = 0; z < model->topics+1; z++) {
        sum += nTM_P_w_z(model, word, z) * nTM_P_z(model, z);
    }
    
    return sum;
}

double nTM_P_z_d(nTM *model, TopicIndex topic, DocumentIndex document)
{
    SparseCounts *ndz = count_list_get(model->ndzs, document);
    unsigned int n_d_z = SparseCounts_getValue(ndz, topic);
    unsigned int n_d_all = model->corpus->documents[document].length;
    WordIndex n_w = model->words;
    TopicIndex n_z = model->topics;
    double alpha = model->alpha;
    double beta = model->beta;
    double gamma = model->gamma;
    
    double numerator = 0;
    if (topic == model->topics) {
        numerator = ((alpha + n_w * beta) * gamma);
    } else {
        numerator = ((n_d_z + alpha + n_w * beta) * (1-gamma));
    }
    
    return numerator / 
           ((n_d_all + n_z * alpha + n_z * n_w * beta) * (1-gamma) + (alpha + n_w * beta) * gamma);
}

double nTM_P_w_z(nTM *model, WordIndex word, TopicIndex topic)
{
    double beta = model->beta;
    WordIndex n_w = model->words;
    if (topic == model->topics) {
        return beta / (n_w * beta);
    }
    
    SparseCounts *nzw = count_list_get(model->nzws, topic);
    unsigned int n_z_w = SparseCounts_getValue(nzw, word);
    unsigned int n_z_all = SparseCounts_getValue(model->nzs, topic);
    
    return (n_z_w + beta) /
           (n_z_all + n_w * beta);
}

WordIndex pzw_last_word = 0;
double pzw_last_pw = -1.0;

double nTM_P_z_w(nTM *model, TopicIndex topic, WordIndex word)
{
    if (pzw_last_pw < 0.0 || pzw_last_word != word) {
        pzw_last_pw = nTM_P_w(model, word);
        pzw_last_word = word;
    }
    
    double value = (nTM_P_w_z(model, word, topic) * nTM_P_z(model, topic)) / pzw_last_pw;
//    printf("P( z = %u | w = %u ) = %f\n",topic,word,value);
    return value;
}

