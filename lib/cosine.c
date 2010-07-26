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
    /*
    double sum = 0.0;
    for (int i=0; i<length; i++) {
        sum += a[i] * b[i];
    }
    
    return sum;
    */
    double s1,s2;
    s1 = s2 = 0.0;
    for (int i=0; i<length; i+=2) {
        s1 += a[i] * b[i];
        if (i+1 < length) {
            s2 += a[i+1] * b[i+1];
        }
    }
    return s1 + s2;
}

double magnitude_ua(unsigned_array *a)
{
		double dot = dot_ua(a,a);
		if(dot == 0.0) {
			printf("Zero magnitude!\n");
			unsigned_array_print(a);
			exit(1);
		}
		return pow(dot,0.5);
}

double cosine_ua(unsigned_array *a, unsigned_array *b)
{
    return dot_ua(a,b) / (magnitude_ua(a) * magnitude_ua(b));
}

double dot_ua(unsigned_array *a, unsigned_array *b)
{
		/*
    double sum = 0.0;
    unsigned int max = a->size > b->size ? a->size : b->size;
    for (int i=0; i<max; i++) {
        sum += unsigned_array_get_zero(a, i) * unsigned_array_get_zero(b, i);
    }
    return sum;
    */
    double s1,s2,s3,s4;
    s1 = s2 = s3 = s4 = 0.0;
    unsigned int max = a->size > b->size ? a->size : b->size;
    for (int i=0; i<max; i+=4) {
        s1 += unsigned_array_get_zero(a, i) * unsigned_array_get_zero(b, i);
        s2 += unsigned_array_get_zero(a, i+1) * unsigned_array_get_zero(b, i+1);
        s3 += unsigned_array_get_zero(a, i+2) * unsigned_array_get_zero(b, i+2);
        s4 += unsigned_array_get_zero(a, i+3) * unsigned_array_get_zero(b, i+3);
    }
    return s1 + s2 + s3 + s4;
}

