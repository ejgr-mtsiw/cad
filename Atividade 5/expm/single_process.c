#include "single_process.h"

int singleProcess(ParsedParams *params, double *a, double **s)
{
    /**
     * M_k matrix
     */
    double *m;

    /**
     * Matrix to hold multiplied values and avoid having to allocate
     * and free memory every time we multiply the matrices
     */
    double *multiplied;

    double *tmp;

    m = (double *)malloc(sizeof(double) * params->n * params->n);
    multiplied = (double *)malloc(sizeof(double) * params->n * params->n);

    if ((*s) == NULL || a == NULL || m == NULL || multiplied == NULL)
    {
        printf("Out of memory!\n");
        return NOK;
    }

    // M1 = A
    memcpy(m, a, sizeof(double) * params->n * params->n);

    // S1 = I + M1
    setIdentityMatrix(s, params->n);
    sumMatrix(m, s, params->n);

    long k = 2;
    do
    {
        // M_k = A * M_k-1 / k
        multiplyMatrix(a, m, &multiplied, params->n);
        memcpy(m, multiplied, sizeof(double) * params->n * params->n);
        divideMatrixByLong(&m, k, params->n);

        // S_k = S_k-1 + M_k
        sumMatrix(m, s, params->n);

        k++;
    } while (maxMij(m, params->n) > params->tolerance);

    free(m);
    free(multiplied);

    return OK;
}
