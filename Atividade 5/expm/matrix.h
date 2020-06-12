#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"

typedef struct matrix
{
    long nRows;
    long nColumns;
    double *data;
} Matrix;

Matrix *createMatrix(long nRows, long nColumns);

Matrix *createMatrixFilledWithZeros(long nRows, long nColumns);

void destroyMatrix(Matrix **m);

Matrix *duplicateMatrix(const Matrix *a);

int copySubMatrix(Matrix **a,
                  const Matrix *b,
                  long startARow,
                  long startAColumn,
                  long startBRow,
                  long startBColumn,
                  long nRows,
                  long nColumns);

double maxMij(const Matrix *m);

int fillMatrixWithRandom(Matrix **a);

int fillMatrixWithZeros(Matrix **a);

int fillArrayWithRandom(double **a, long n);

int fillArrayWithZeros(double **a, long n);

int sumMatrix(const Matrix *m, Matrix **s);

/**
 * Fills the matrix with the Identity Matrix starting in
 * startRow and startColumn
 * 
 * s = [x x]
 *     [x x]
 * startRow = 1, startColumn = 0
 * s = [0 1]
 *     [0 0]
 * 
 * -------------
 * s = [x x x x]
 * startRow = 2, startColumn = 0
 * s = [0 0 1 0]
 */
int setIdentitySubMatrix(Matrix **s, long startRow, long startColumn);

int setIdentityMatrix(Matrix **s);

/**
 * Multiplies two matrices and replaces the values on the multiplied matrix.
 */
int multiplyMatrix(const Matrix *a, const Matrix *b, Matrix **multiplied);

/**
 * Multiplies two matrices and adds the result to the multiplied matrix
 */
int multiplyMatrixAndSum(const Matrix *a, const Matrix *b, Matrix **multiplied);

/**
 * Multiplies two matrices using the method presented in PPC (p.276)
 */
int multiplyMatrixAndSumBlock(const Matrix *a,
                              const Matrix *b,
                              Matrix **multiplied,
                              int arow,
                              int acol,
                              int brow,
                              int bcol,
                              int crow,
                              int ccol,
                              int l,
                              int m,
                              int n);

int divideMatrixByLong(Matrix **a, long number);

void printMatrix(const char *name, const Matrix *m, int format);

int printMatrixToFile(const char *filename, const char *name, const Matrix *m, int format, int append);

void writeMatrix(FILE *fp, const char *name, const Matrix *m, int format);

#endif
