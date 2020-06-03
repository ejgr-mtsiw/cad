#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdlib.h>
#include <stdio.h>

#include "util.h"

double maxMij(const double *m, long n);

int fillWithRandom(double **a, long n);

int sumMatrix(const double *m, double **s, long n);

int setIdentityMatrix(double **s, long n);

int multiplyMatrix(const double *a, const double *b, double **multiplied, long n);

int divideMatrixByLong(double **a, long number, long n);

void printMatrix(const char *name, const double *m, long n, int format);

int printMatrixToFile(const char *filename, const char *name, const double *m, long n, int format, int append);

#endif
