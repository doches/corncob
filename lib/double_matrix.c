/*
 *  double_matrix.c
 *  corncob
 *
 *  Created by Trevor Fountain on 7/15/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "double_matrix.h"
#include <stdio.h>

double_matrix *double_matrix_new(unsigned int rows, unsigned int cols, double zero)
{
    double_matrix *new = (double_matrix *)malloc(sizeof(double_matrix));
    new->rows = 0;
    new->cols = 0;
    new->max_rows = rows;
    new->max_cols = cols;
    new->zero = zero;
    new->data = (double **)malloc(sizeof(double *)*rows);
    for (int i=0; i<rows; i++) {
        new->data[i] = (double *)malloc(sizeof(double)*cols);
        memset(new->data[i],zero,cols);
    }
    return new;
}

double *double_matrix_get_pointer(double_matrix *matrix, unsigned int row, unsigned int col)
{
    if (col >= matrix->max_cols) {
        unsigned int new_cols = matrix->max_cols;
        while (col >= new_cols) {
            new_cols *= 2;
        }
        for (int i=0; i<matrix->max_rows; i++) {
            double *new_row = (double *)malloc(sizeof(double)*new_cols);
            memset(new_row,matrix->zero,sizeof(double)*new_cols);
            memcpy(new_row,matrix->data[i],sizeof(double)*matrix->max_cols);
            free(matrix->data[i]);
            matrix->data[i] = new_row;
        }
        matrix->max_cols = new_cols;
    }
    
    if (row >= matrix->max_rows) {
        unsigned int new_rows = matrix->max_rows;
        while (row >= new_rows) {
            new_rows *= 2;
        }
        double **new_data = (double **)malloc(sizeof(double *)*new_rows);
        memcpy(new_data,matrix->data,sizeof(double *)*matrix->max_rows);
        free(matrix->data);
        matrix->data = new_data;
        for (int i=matrix->max_rows; i<new_rows; i++) {
            matrix->data[i] = (double *)malloc(sizeof(double)*matrix->max_cols);
            memset(matrix->data[i],matrix->zero,sizeof(double)*matrix->max_cols);
        }
        
        matrix->max_rows = new_rows;
    }
    
    if (row+1 > matrix->rows) {
        matrix->rows = row+1;
    }
    if (col+1 > matrix->cols) {
        matrix->cols = col+1;
    }
    return &(matrix->data[row][col]);
}

void double_matrix_set(double_matrix *matrix, unsigned int row, unsigned int col, double value)
{
    *double_matrix_get_pointer(matrix, row, col) = value;
}

double double_matrix_get(double_matrix *matrix, unsigned int row, unsigned int col)
{
    return *double_matrix_get_pointer(matrix, row, col);
}

double double_matrix_get_zero(double_matrix *matrix, unsigned int row, unsigned int col)
{
    if (row > matrix->max_rows || col > matrix->max_cols) {
        return matrix->zero;
    }
    return double_matrix_get(matrix,row,col);
}

void double_matrix_free(double_matrix *matrix)
{
    for (int i=0; i<matrix->rows; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

void double_matrix_print(double_matrix *matrix)
{
    for (int i=0; i<matrix->rows; i++) {
        for (int j=0; j<matrix->cols; j++) {
            printf("%f ",matrix->data[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

