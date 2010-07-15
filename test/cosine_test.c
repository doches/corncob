/*
 *  cosine_test.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "cosine_test.h"
#include "unsigned_array.h"
#include <math.h>

void test_cosine()
{
    unsigned int a[3] = {4,5,6};
    assert(dot(a,a,3) == (4*4+5*5+6*6));
    assert(magnitude(a, 3) == pow(77.0,0.5));
    
    unsigned_array *ua = unsigned_array_new(16);
    unsigned_array_set(ua, 0, 4);
    unsigned_array_set(ua, 1, 5);
    unsigned_array_set(ua, 2, 6);
    assert(dot_ua(ua,ua) == (4*4+5*5+6*6));
    assert(magnitude_ua(ua) == pow(77.0,0.5));
}

