/*
 *  unsigned_array.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "unsigned_array.h"
#include <stdio.h>

unsigned_array *unsigned_array_new(unsigned int max)
{
    unsigned_array *new = (unsigned_array *)malloc(sizeof(unsigned_array));
    new->max = max;
    new->data = (unsigned int *)malloc(sizeof(unsigned int)*(new->max));
    memset(new->data,0x0,sizeof(unsigned int)*new->max);
    return new;
}

unsigned int *unsigned_array_get_pointer(unsigned_array *uarray, unsigned int index)
{
    if (index >= uarray->max) {
        unsigned int new_max = uarray->max*2;
        while (index >= new_max) {
            new_max *= 2;
        }
        unsigned int *new_data = (unsigned int *)malloc(sizeof(unsigned int)*new_max);
        memset(new_data,0x0,sizeof(unsigned int)*new_max);
        memcpy(new_data,uarray->data,sizeof(unsigned int)*uarray->max);
        free(uarray->data);
        uarray->data = new_data;
    }
    return &(uarray->data[index]);
}

unsigned int unsigned_array_get(unsigned_array *uarray, unsigned int index)
{
    return *unsigned_array_get_pointer(uarray, index);
}

unsigned int unsigned_array_get_zero(unsigned_array *uarray, unsigned int index)
{
    if (index < uarray->max) {
        return uarray->data[index];
    } 
    
    return 0;
}

void unsigned_array_set(unsigned_array *uarray, unsigned int index, unsigned int value)
{
    if (index+1 > uarray->size) {
        uarray->size = index+1;
    }
    *(unsigned_array_get_pointer(uarray, index)) = value;
}

void unsigned_array_add(unsigned_array *uarray, unsigned int index, unsigned int value)
{
    if (index+1 > uarray->size) {
        uarray->size = index+1;
    }
    *(unsigned_array_get_pointer(uarray, index)) += value;
}

void unsigned_array_print(unsigned_array *uarray)
{
    for (int i=0; i<uarray->size; i++) {
        printf("%d ",uarray->data[i]);
    }
    printf("(%d/%d)",uarray->size,uarray->max);
}

void unsigned_array_free(unsigned_array *uarray)
{
    free(uarray->data);
    free(uarray);
}

