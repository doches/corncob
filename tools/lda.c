/*
 *  lda.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "assert.h"
#include "lda.h"

LDA *model;

int main()
{
	srand(time(NULL));
	
	corpus *bnc = corpus_new("bnc", "xml",20);
	unsigned int n_topics = 5;
	model = LDA_new(bnc->size, // # documents
						 n_topics, // # topics
						 20000, // word buckets
						 n_topics, // topic buckets
						 100, // document buckets
						 50.0/n_topics, // alpha
						 0.1f); // beta
	corpus_each_document(bnc, &LDA_train_document);
	printf("\n");
	
	printf("Read %d words in %d documents\n",model->words->size,bnc->size);
	LDA_dump_wordmap(model,"bnc.wordmap");
	
	LDA_trim_assignments(model,2);
	
	for (int i=0; i<1001; i++) {
		printf("Gibbs Iteration %d:\n",i);
		LDA_gibbs(model);
		if(i%100 == 0) {
		  char fname[15];
		  sprintf(fname,"bnc.%d.counts",i);
		  LDA_dump_counts(model,fname);
		}
	}
	
	LDA_delete(model);
	corpus_free(bnc);
	return 0;
}

void LDA_trim_assignments(LDA *model, unsigned int threshold)
{
  unsigned int *word_counts = malloc(sizeof(unsigned int)*(model->words->size));
  memset(word_counts,0,sizeof(unsigned int)*(model->words->size));
  Instance *assign = model->assignments;
  unsigned int total = 0;
  while(assign)
  {
    word_counts[assign->w_i]++;
    assign = assign->next;
    total++;
  }
  assign = model->assignments;
  Instance *prev = NULL;
  unsigned int culled = 0;
  while(assign)
  {
    if(word_counts[assign->w_i] < threshold) {
      Instance *old = assign;
      if(prev != NULL) {
        prev->next = assign->next;
      } else {
        model->assignments = assign->next;
      }
      assign = assign->next;
      free(old);
      culled++;
    } else {
      prev = assign;
      assign = assign->next;
    }
  }
  printf("Trimming %d low-frequency words (%d remaining)\n",culled,total-culled);
}

double LDA_p_topic(LDA *model, unsigned int t_index, unsigned int w_index)
{
//	double a = (hash_get(model->topics[t_index],w_index)->value + model->beta)/(double)(hash_get(model->topic_counts,t_index)->value + (model->words->size+1)*model->beta);
//	double b = (hash_get(model->documents[model->document_index],t_index)->value + model->alpha)/(double)(hash_get(model->document_counts,model->document_index)->value + model->n_topics*model->alpha);
	double a = (hash_get(model->topics[t_index],w_index)->value + model->beta)/(double)(model->topic_counts[t_index] + (model->words->size+1)*model->beta);
	double b = (hash_get(model->documents[model->document_index],t_index)->value + model->alpha)/(double)(model->document_counts[model->document_index] + model->n_topics*model->alpha);
	return a*b;
}

unsigned int LDA_sample_topic(LDA *model,unsigned int w_index)
{
	double *scores = malloc(sizeof(double)*model->n_topics);
	double sum = 0;
//	double a_den = (model->words->size+1)*model->beta;
//  double b_den = model->document_counts[model->document_index] + model->n_topics*model->alpha;
	for (int t_i=0; t_i<model->n_topics; t_i++) {
		scores[t_i] = LDA_p_topic(model, t_i, w_index);
//    double a = (hash_get(model->topics[t_i],w_index)->value + model->beta)/(model->topic_counts[t_i] + a_den);
//	  double b = (hash_get(model->documents[model->document_index],t_i)->value + model->alpha)/b_den;
//	  scores[t_i] = a*b;
		sum += scores[t_i];
	}
	float r = ((rand()%10000)/10000.0f)*sum;
	for (int t_i=0; t_i<model->n_topics; t_i++) {
		r -= scores[t_i];
		if (r <= 0.0f) {
			return t_i;
		}
	}
	printf("Oversampled!\n");
	exit(1);
	return scores[model->n_topics-1];
}

word_hash_element *LDA_word_index(LDA *model,char *word)
{
	word_hash_element *w_elem = word_hash_get(model->words,word);
/*
	if(word[strlen(word)-1] == ' ') {
	  word[strlen(word)-1] = '\0';
	}
	*/
	if (w_elem->key != NULL) {
	  //printf("[%s] has index %d\n",w_elem->key,w_elem->value);
		return w_elem;
	} else {
		w_elem = word_hash_add(model->words,word,-1);
		w_elem->value = model->words->size-1;
		//printf("[%s] get index %d\n",w_elem->key,w_elem->value);
		return w_elem;
	}
}

void LDA_train_word(LDA *model,char *word)
{	
	word_hash_element *w_item = LDA_word_index(model, word);
	unsigned int w_index = w_item->value;
	unsigned int t_index = LDA_sample_topic(model, w_index);
	
	// Increment count of this document overall
	model->document_counts[model->document_index]++;
	// Increment count of this topic overall
	model->topic_counts[t_index]++;
	// Increment count of sampled topic in this document
	hash_element *doc = hash_get(model->documents[model->document_index],t_index);
	if (doc->key == -1) {
		doc = hash_add(model->documents[model->document_index], t_index, 0);
		doc->value = 0;
	}
	doc->value++;
	// Increment count of word in sampled topic
	hash_element *topic = hash_get(model->topics[t_index],w_index);
	if (topic->key == -1) {
		topic = hash_add(model->topics[t_index],w_index,0);
		topic->value = 0;
	}
	topic->value++;
	
	Instance *assignment = Instance_new(w_index,t_index,model->document_index,0,w_item->key);
	if (model->assignments) {
		assignment->next = model->assignments;
		assignment->index = model->assignments->index+1;
	}
	model->assignments = assignment;
}

void LDA_gibbs(LDA *model)
{
	Instance *a = model->assignments;
	while (a) {
		model->document_index = a->d_i;

		hash_element *e = hash_get(model->topics[a->z_i],a->w_i);
		if(e->key == -1) {
		  printf("Bad topic");
		  exit(1);
		}
//		printf("topic %d, word %s = %d\n",a->z_i,a->word,e->value);
		e->value--;
//		printf("topic %d, word %s = %d\n",a->z_i,a->word,e->value);
		e = hash_get(model->documents[a->d_i], a->z_i);
		if(e->key == -1) {
		  printf("bad doc");
		  exit(1);
		}
		e->value--;
    model->topic_counts[a->z_i]--;
		model->document_counts[a->d_i]--;
		
		unsigned int new_z = LDA_sample_topic(model, a->w_i);
		
		e = hash_get(model->topics[new_z],a->w_i);
		if(e->key == -1) {
		  e = hash_add(model->topics[new_z],a->w_i,0);
		  e->value = 0;
		}
//		printf("topic %d, word %s = %d\n",new_z,a->word,e->value);
		e->value++;
//		printf("topic %d, word %s = %d\n",new_z,a->word,e->value);
		
		e = hash_get(model->documents[a->d_i], new_z);
		if(e->key == -1) {
		  e = hash_add(model->documents[a->d_i],new_z,0);
		}
		e->value++;
		
    model->topic_counts[new_z]++;
		model->document_counts[a->d_i]++;
		
		if(a->z_i != new_z) {
		  //printf("%s: %d -> %d\n",a->word,a->z_i,new_z);
		}
		a->z_i = new_z;
		
		a = a->next;
	}
}

void each_word(char *word)
{
	LDA_train_word(model, word);
}

void LDA_train_document(char *path)
{
	document_each_word(path, &each_word);
	model->document_index++;
	printf("Reading %s\n",path);
}

LDA *LDA_new(int n_docs, int n_topics, int w_buckets, int t_buckets, int d_buckets,double alpha, double beta)
{
	LDA *model = malloc(sizeof(LDA));
	
	model->n_documents = n_docs;
	model->documents = malloc(sizeof(ct_hash*)*n_docs);
	for (int d_i=0; d_i < n_docs; d_i++) {
		model->documents[d_i] = hash_new(d_buckets);
	}
	//model->document_counts = hash_new(n_docs);
	model->document_counts = malloc(sizeof(unsigned int)*(n_docs));
	for(int d_i=0;d_i < n_docs;d_i++) {
	  model->document_counts[d_i] = 0;
	}
	
	model->n_topics = n_topics;
	model->topics = malloc(sizeof(ct_hash*)*n_topics);
	for (int t_i=0; t_i < n_topics; t_i++) {
		model->topics[t_i] = hash_new(t_buckets);
	}
	//model->topic_counts = hash_new(n_topics);
	model->topic_counts = malloc(sizeof(unsigned int)*(n_topics));
	for(int t_i=0;t_i<n_topics;t_i++) {
	  model->topic_counts[t_i] = 0;
	}
	
	model->words = word_hash_new(w_buckets);
	
	model->alpha = alpha;
	model->beta = beta;

	model->document_index = 0;
	model->assignments = NULL;
	return model;
}

void LDA_delete(LDA *model)
{
	for(int d_i=0;d_i<model->n_documents;d_i++) {
		free(model->documents[d_i]);
	}
	free(model->documents);
	free(model->document_counts);
	for(int t_i=0;t_i<model->n_topics;t_i++) {
		free(model->topics[t_i]);
	}
	free(model->topics);
	free(model->topic_counts);
	free(model->words);
	free(model->assignments);
	free(model);
}

FILE *fout;

void dump_word(word_hash_element *word_e)
{
  char index[20];
  sprintf(index,"%d",word_e->value);
  char *str = malloc(sizeof(char)*(strlen(index)+2+strlen(word_e->key)));
  strcpy(str,index);
  strcat(str," ");
  strcat(str,word_e->key);
  fputs(str,fout);
  fputs("\n",fout);
  free(str);
}

void LDA_dump_wordmap(LDA *model,char *filename)
{
  fout = fopen(filename,"w");
  word_hash_foreach(model->words,&dump_word);
  fclose(fout);
}

void dump_topic(hash_element *e)
{
  char str[30];
  sprintf(str,"%d_%d ",e->key,e->value);
  fputs(str,fout);
//  fputs(e->key,fout);
//  fputs("_",fout);
//  fputs(e->value,fout);
}

void LDA_dump_counts(LDA *model,char *filename)
{
  fout = fopen(filename,"w");
  for(int i=0;i<model->n_topics;i++)
  {
    hash_foreach(model->topics[i],&dump_topic);
    fputs("\n",fout);
//    printf("%d ",model->topic_counts[i]);
  }
//  printf("\n");
  fclose(fout);
}
