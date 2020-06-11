#include "single_process.h"

int singleProcess(const ParsedParams *params, const Matrix *a, Matrix **s)
{
    /**
     * M_k matrix
     */
    Matrix *m;

    /**
     * Matrix to hold multiplied values and avoid having to allocate
     * and free memory every time we multiply the matrices
     */
    Matrix *multiplied = createMatrix(params->n, params->n);

    long d = params->n * params->n;

    double *zeroes = (double *)malloc(sizeof(double) * d);
    fillArrayWithZeros(&zeroes, d);

    /**
     * Temporary pointer for data switch
     */
    double *tmp;

    // M1 = A
    m = duplicateMatrix(a);

    // S1 = I + M1
    setIdentityMatrix(s);
    sumMatrix(m, s);

    long k = 2;
    do
    {
        // M_k = A * M_k-1 / k
        memcpy(multiplied->data, zeroes, sizeof(double) * d);

        multiplyMatrixAndSumBlock(a,
                                  m,
                                  &multiplied,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  a->nRows,
                                  m->nRows,
                                  m->nColumns);

        tmp = multiplied->data;
        multiplied->data = m->data;
        m->data = tmp;

        divideMatrixByLong(&m, k);

        // S_k = S_k-1 + M_k
        sumMatrix(m, s);

        k++;
    } while (maxMij(m) > params->tolerance);

    destroyMatrix(&m);
    destroyMatrix(&multiplied);

    return OK;
}
