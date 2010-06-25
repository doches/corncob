#include "count_list.h"

count_list *count_list_new()
{
	count_list *new = malloc(sizeof(count_list));
	new->size = COUNT_LIST_DEFAULT_SIZE;
	new->counts = malloc(sizeof(SparseCounts *) * new->size);
	new->used = 0;
	for(int i=0;i<new->size;i++) {
		new->counts[i] = NULL;
	}
	return new;
}

void count_list_free(count_list *list)
{
	for(int i=0;i<list->used;i++) {
		SparseCounts_free(list->counts[i]);
	}
	
	free(list->counts);
	free(list);
}

void count_list_resize(count_list *list)
{
	list->size *= 2;
	SparseCounts **new_counts = malloc(sizeof(SparseCounts *) * list->size);
	for (int i=list->used; i<list->size; i++) {
		new_counts[i] = NULL;
	}
	for(int i=0;i<list->used;i++) {
		new_counts[i] = list->counts[i];
	}
	SparseCounts **old_counts = list->counts;
	list->counts = new_counts;
	free(old_counts);
}

SparseCounts *count_list_get(count_list *list, unsigned int index)
{
	if (index >= list->size) {
		return NULL;
	}
	return list->counts[index];
}

void count_list_add(count_list *list)
{
	if(list->used >= list->size) {
		count_list_resize(list);
	}
	list->counts[list->used++] = SparseCounts_new(SPARSECOUNTS_BUCKETS);
}

void count_list_set(count_list *list,SparseCounts *counts, unsigned int index)
{
	while (index >= list->size-1) {
		count_list_resize(list);
	}
	if (list->counts[index]==NULL) {
		list->used++;
	}
	list->counts[index] = counts;
}

