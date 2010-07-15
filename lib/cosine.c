/*
 *  cosine.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "cosine.h"
#include <math.h>
#include <stdio.h>

double magnitude(unsigned int *a, unsigned int length)
{
    return pow(dot(a,a,length),0.5);
}

double cosine(unsigned int *a, unsigned int *b, unsigned int length)
{
    return dot(a,b,length) / (magnitude(a, length) * magnitude(b, length));
}

double dot(unsigned int *a, unsigned int *b, unsigned int length)
{
    double sum = 0.0;
    for (int i=0; i<length; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

double magnitude_ua(unsigned_array *a)
{
    return pow(dot_ua(a,a),0.5);
}

double cosine_ua(unsigned_array *a, unsigned_array *b)
{
    return dot_ua(a,b) / (magnitude_ua(a) * magnitude_ua(b));
}

double dot_ua(unsigned_array *a, unsigned_array *b)
{
    double sum = 0.0;
    unsigned int max = a->size > b->size ? a->size : b->size;
    for (int i=0; i<max; i++) {
        sum += unsigned_array_get_zero(a, i) * unsigned_array_get_zero(b, i);
    }
    return sum;
}

