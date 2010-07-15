/*
 *  unsigned_array.h
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#ifndef UNSIGNED_ARRAY_H
#define UNSIGNED_ARRAY_H

#include <stdlib.h>
#include <string.h>

typedef struct unsigned_array_t
{
    unsigned int *data;
    unsigned int max;
    unsigned int size;
} unsigned_array;

unsigned_array *unsigned_array_new(unsigned int max);
unsigned int *unsigned_array_get_pointer(unsigned_array *uarray, unsigned int index);
unsigned int unsigned_array_get(unsigned_array *uarray, unsigned int index);
unsigned int unsigned_array_get_zero(unsigned_array *uarray, unsigned int index);
void unsigned_array_add(unsigned_array *uarray, unsigned int index, unsigned int value);
void unsigned_array_set(unsigned_array *uarray, unsigned int index, unsigned int value);
void unsigned_array_free(unsigned_array *uarray);
void unsigned_array_print(unsigned_array *uarray);

#endif

