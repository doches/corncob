/*
 *  double_matrix_test.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/15/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "double_matrix_test.h"
#include <math.h>

void test_double_matrix()
{
		const double epsilon = 0.0001;
		
    double_matrix *matrix = double_matrix_new(4,4,0.0);
    
    assert(abs(double_matrix_get_zero(matrix, 0, 0)) < epsilon);
    double_matrix_set(matrix, 0, 0, 1.0);
    assert(1.0 == double_matrix_get_zero(matrix, 0, 0));
    double_matrix_set(matrix, 8, 8, 2.0);
    assert(2.0 == double_matrix_get_zero(matrix, 8, 8));
    assert(1.0 == double_matrix_get_zero(matrix, 0, 0));
    double_matrix_set(matrix, 8, 7, 30.0);
    assert(1.0 == double_matrix_get_zero(matrix, 0, 0));
    assert(2.0 == double_matrix_get_zero(matrix, 8, 8));
    assert(30.0 == double_matrix_get_zero(matrix, 8, 7));
    
    for (int i=0; i<8; i++) {
        double_matrix_set(matrix, 0, i, i+1);
    }
    printf("\n");
    double_matrix_print(matrix);
    
    double_matrix_free(matrix);
}

