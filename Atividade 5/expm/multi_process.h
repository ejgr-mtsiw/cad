#ifndef __MULTI_PROCESS_H__
#define __MULTI_PROCESS_H__

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"
#include "parse_param.h"

int multiProcess(ParsedParams *params,
                 const Matrix *globalA,
                 Matrix **globalS,
                 int myrank,
                 int npes);

/**
 * Calculates the number of columns to use.
 * If n is not a multiple of npes we add new columns filled with zeroes
 */
long calculateColumnsPerProcess(long n, int npes);

/**
 * Shares the A matrix using MPI_Scatterv
 * If necessary (n!=nColumnsPerProcess) the values are adjusted for the
 * new internal dimensions
 */
int shareA(const Matrix *globalA, Matrix **a, int myrank, int npes);

/**
 * Builds the final S Matrix using the s data from each subprocess
 * If necessary (n!=nColumnsPerProcess) the values are adjusted to the
 * original dimensions
 */
int buildFinalSMatrix(Matrix **globalS, Matrix *s, int myrank, int npes);

#endif
