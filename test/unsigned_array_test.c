/*
 *  unsigned_array_test.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/13/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "unsigned_array_test.h"

void test_unsigned_array()
{
    unsigned_array *uarray = unsigned_array_new(16);
    assert(unsigned_array_get(uarray, 0) == 0);
    assert(unsigned_array_get(uarray, 16) == 0);
    assert(unsigned_array_get_zero(uarray, 32) == 0);
    unsigned_array_set(uarray, 0, 32);
    assert(unsigned_array_get(uarray, 0) == 32);
    assert(unsigned_array_get_zero(uarray, 0) == 32);
    assert(unsigned_array_get(uarray, 128) == 0);
    assert(unsigned_array_get(uarray, 0) == 32);
    assert(unsigned_array_get_zero(uarray, 0) == 32);
    unsigned_array_add(uarray, 0, 1);
    assert(unsigned_array_get(uarray, 0) == 33);
    assert(unsigned_array_get_zero(uarray, 0) == 33);
}