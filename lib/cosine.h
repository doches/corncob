/*
 *  cosine.h
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */
#include "unsigned_array.h"

double dot(unsigned int *a, unsigned int *b, unsigned int length);
double magnitude(unsigned int *a, unsigned int length);
double cosine(unsigned int *a, unsigned int *b, unsigned int length);

double cosine_ua(unsigned_array *a, unsigned_array *b);
double magnitude_ua(unsigned_array *a);
double dot_ua(unsigned_array *a, unsigned_array *b);

unsigned int manhattan(unsigned_array *a, unsigned_array *b);

