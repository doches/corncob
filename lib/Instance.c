/*
 *  lda.c
 *  ctools
 *
 *  Created by Trevor Fountain on 3/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "Instance.h"
#include <stdio.h>

Instance *Instance_new(unsigned int w_i,unsigned int z_i, unsigned int d_i, unsigned int index,char *word)
{
	Instance *new = malloc(sizeof(Instance));
	new->w_i = w_i;
	new->z_i = z_i;
	new->d_i = d_i;
	new->index = index;
	new->word = word;
	new->next = NULL;
	return new;
}

void Instance_free(Instance *list)
{
	Instance *next;
	while(list != NULL)
	{
		next = list->next;
		free(list);
		list = next;
	}
}

void Instance_print(Instance *i)
{
	printf("[%3d: %3d %3d %3d %-10.10s]\n",i->index,i->w_i,i->z_i,i->d_i,i->word);
}
