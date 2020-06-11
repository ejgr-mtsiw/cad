#include "matrix.h"

Matrix *createMatrix(long nRows, long nColumns)
{
    Matrix *m;

    m = (Matrix *)malloc(sizeof(Matrix));
    m->nColumns = nColumns;
    m->nRows = nRows;

    m->data = (double *)malloc(sizeof(double) * nRows * nColumns);

    return m;
}

Matrix *createMatrixFilledWithZeros(long nRows, long nColumns)
{
    Matrix *m = createMatrix(nRows, nColumns);

    for (long i = 0; i < nRows * nColumns; i++)
    {
        m->data[i] = 0.0;
    }

    return m;
}

void destroyMatrix(Matrix **m)
{
    if ((*m) == NULL)
    {
        return;
    }

    if ((*m)->data != NULL)
    {
        free((*m)->data);
    }

    free(*m);
}

Matrix *duplicateMatrix(const Matrix *a)
{
    Matrix *m = createMatrix(a->nRows, a->nColumns);
    memcpy(m->data, a->data, sizeof(double) * a->nRows * a->nColumns);
    return m;
}

int copySubMatrix(Matrix **a, const Matrix *b, long startARow, long startAColumn, long startBRow, long startBColumn, long nRows, long nColumns)
{
    for (long i = 0; i < nRows && i + startBRow < b->nRows && i + startARow < (*a)->nRows; i++)
    {
        for (long j = 0; j < nColumns && j + startBColumn < b->nColumns && j + startAColumn < (*a)->nColumns; j++)
        {
            (*a)->data[(i + startARow) * (*a)->nColumns + (j + startAColumn)] = b->data[(i + startBRow) * b->nColumns + (j + startBColumn)];
        }
    }

    return OK;
}

double maxMij(const Matrix *m)
{
    double max = fabs(m->data[0]);
    double v = 0.0;

    for (long i = 0; i < m->nRows; i++)
    {
        for (long j = 0; j < m->nColumns; j++)
        {
            v = fabs(m->data[i * m->nColumns + j]);
            if (v > max)
            {
                max = v;
            }
        }
    }

    return max;
}

int fillMatrixWithRandom(Matrix **a)
{
    return fillArrayWithRandom(&((*a)->data), (*a)->nColumns * (*a)->nRows);
}

int fillMatrixWithZeros(Matrix **a)
{
    return fillArrayWithZeros(&((*a)->data), (*a)->nColumns * (*a)->nRows);
}

int fillArrayWithRandom(double **a, long n)
{
    for (long i = 0; i < n; i++)
    {
        (*a)[i] = RANDOM_VALUE;
    }
    return OK;
}

int fillArrayWithZeros(double **a, long n)
{
    for (long i = 0; i < n; i++)
    {
        (*a)[i] = 0.0;
    }
    return OK;
}

int sumMatrix(const Matrix *m, Matrix **s)
{
    long nColumns = (*s)->nColumns;
    long nRows = (*s)->nRows;

    for (long i = 0; i < nRows; i++)
    {
        for (long j = 0; j < nColumns; j++)
        {
            (*s)->data[i * nColumns + j] += m->data[i * nColumns + j];
        }
    }

    return OK;
}

int setIdentityMatrix(Matrix **s)
{
    return setIdentitySubMatrix(s, 0, 0);
}

int setIdentitySubMatrix(Matrix **s, long startRow, long startColumn)
{
    long pos = 0;
    long nRows = (*s)->nRows;
    long nColumns = (*s)->nColumns;

    for (long i = 0; i < nRows; i++)
    {
        for (long j = 0; j < nColumns; j++)
        {
            if (i + startRow == j + startColumn)
            {
                (*s)->data[i * nColumns + j] = 1.0;
            }
            else
            {
                (*s)->data[i * nColumns + j] = 0.0;
            }
        }
    }

    return OK;
}

int multiplyMatrix(const Matrix *a, const Matrix *b, Matrix **multiplied)
{
    double val;

    if (a->nColumns != b->nRows)
    {
        return NOK;
    }

    for (long i = 0; i < a->nRows; i++)
    {
        for (long j = 0; j < b->nColumns; j++)
        {
            val = 0.0;
            for (long k = 0; k < a->nColumns; k++)
            {
                val += a->data[i * a->nColumns + k] * b->data[k * b->nColumns + j];
            }

            (*multiplied)->data[i * b->nColumns + j] = val;
        }
    }

    return OK;
}

int multiplyMatrixAndSum(const Matrix *a, const Matrix *b, Matrix **multiplied)
{
    double val;

    if (a->nColumns != b->nRows)
    {
        return NOK;
    }

    for (long i = 0; i < a->nRows; i++)
    {
        for (long j = 0; j < b->nColumns; j++)
        {
            val = 0.0;
            for (long k = 0; k < a->nColumns; k++)
            {
                val += a->data[i * a->nColumns + k] * b->data[k * b->nColumns + j];
            }

            (*multiplied)->data[i * b->nColumns + j] += val;
        }
    }

    return OK;
}

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
                              int n)
{
    double *aptr, *bptr, *cptr;

    int lhalf[3], mhalf[3], nhalf[3]; // Quadrant sizes
    int i, j, k;

    if (m * n > CACHE_THRESHOLD)
    {
        /* B doesn't fit in cache --- multiply blocks of A, B*/

        lhalf[0] = 0;
        lhalf[1] = l / 2;
        lhalf[2] = l - l / 2;

        mhalf[0] = 0;
        mhalf[1] = m / 2;
        mhalf[2] = m - m / 2;

        nhalf[0] = 0;
        nhalf[1] = n / 2;
        nhalf[2] = n - n / 2;

        for (i = 0; i < 2; i++)
        {
            for (j = 0; j < 2; j++)
            {
                for (k = 0; k < 2; k++)
                {
                    multiplyMatrixAndSumBlock(a,
                                              b,
                                              multiplied,
                                              arow + lhalf[i],
                                              acol + mhalf[k],
                                              brow + mhalf[k],
                                              bcol + nhalf[j],
                                              crow + lhalf[i],
                                              ccol + nhalf[j],
                                              lhalf[i + 1],
                                              mhalf[k + 1],
                                              nhalf[j + 1]);
                }
            }
        }
    }
    else
    {
        for (long i = 0; i < l; i++)
        {
            for (long j = 0; j < n; j++)
            {
                aptr = &a->data[(arow + i) * a->nColumns + acol];
                bptr = &b->data[brow * b->nColumns + (bcol + j)];
                cptr = &(*multiplied)->data[(crow + i) * (*multiplied)->nColumns + (ccol + j)];

                for (long k = 0; k < m; k++)
                {
                    *cptr += *(aptr++) * *bptr;
                    bptr += b->nColumns;
                }
            }
        }
    }

    return OK;
}

int divideMatrixByLong(Matrix **a, long number)
{
    long nRows = (*a)->nRows;
    long nColumns = (*a)->nColumns;

    for (long i = 0; i < nRows; i++)
    {
        for (long j = 0; j < nColumns; j++)
        {
            (*a)->data[i * nColumns + j] = (*a)->data[i * nColumns + j] / number;
        }
    }

    return OK;
}

void printMatrix(const char *name, const Matrix *m, int format)
{
    writeMatrix(stdout, name, m, format);
}

int printMatrixToFile(const char *filename, const char *name, const Matrix *m, int format, int append)
{
    const char *formatString;
    FILE *outputFile;

    if (append == 1)
    {
        outputFile = fopen(filename, "a");
    }
    else
    {
        outputFile = fopen(filename, "w");
    }

    if (outputFile == NULL)
    {
        printf("[ERROR] Error opening file for writing!\n");
        return NOK;
    }

    writeMatrix(outputFile, name, m, format);

    fclose(outputFile);

    return OK;
}

void writeMatrix(FILE *fp, const char *name, const Matrix *m, int format)
{
    const char *formatString;

    if (format == USE_LONG_FORMAT)
    {
        formatString = LONG_FORMAT;
    }
    else
    {
        formatString = SHORT_FORMAT;
    }

    if (name != NULL)
    {
        fprintf(fp, "\n%s=[\n", name);
    }

    for (long i = 0; i < m->nRows && i < MAX_ROWS_TO_OUTPUT; i++)
    {
        for (long j = 0; j < m->nColumns && j < MAX_COLUMNS_TO_OUTPUT; j++)
        {
            fprintf(fp, formatString, m->data[i * m->nColumns + j]);
        }

        if (m->nColumns > MAX_COLUMNS_TO_OUTPUT)
        {
            fprintf(fp, " ...");
        }
        fprintf(fp, "\n");
    }

    if (m->nRows > MAX_ROWS_TO_OUTPUT)
    {
        fprintf(fp, " ...\n");
    }
    fprintf(fp, "];\n");
}