#include "count_list.h"

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

SparseCounts *count_list_get(count_list *start, unsigned int max)
{
	int current = 0;
	count_list *counts = start;
	while(counts != NULL && current < max) {
		counts = counts->next;
		current++;
	}
	if(counts != NULL) {
		return counts->counts;
	} else {
		return NULL;
	}
}
