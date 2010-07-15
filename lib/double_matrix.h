/*
 *  double_matrix.h
 *  corncob
 *
 *  Created by Trevor Fountain on 7/15/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

typedef struct _double_matrix_t
{
    unsigned int max_rows;
    unsigned int rows;
    unsigned int max_cols;
    unsigned int cols;
    double **data;
    double zero;
} double_matrix;

double_matrix *double_matrix_new(unsigned int rows, unsigned int cols, double zero);
double *double_matrix_get_pointer(double_matrix *matrix, unsigned int row, unsigned int col);
void double_matrix_set(double_matrix *matrix, unsigned int row, unsigned int col, double value);
double double_matrix_get(double_matrix *matrix, unsigned int row, unsigned int col);
double double_matrix_get_zero(double_matrix *matrix, unsigned int row, unsigned int col);
void double_matrix_free(double_matrix *matrix);
void double_matrix_print(double_matrix *matrix);