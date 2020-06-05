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

int copySubMatrix(Matrix **a, const Matrix *b, long startRow, long nRows, long startColumn, long nColumns)
{
    for (long i = 0; i < nRows && i < b->nRows; i++)
    {
        for (long j = 0; j < nColumns && j < b->nColumns; j++)
        {
            (*a)->data[i * (*a)->nColumns + j] = b->data[(i + startRow) * b->nColumns + j + startColumn];
        }
    }

    return OK;
}

double maxMij(const Matrix *m)
{
    double max = m->data[0];

    for (long i = 0; i < m->nRows; i++)
    {
        for (long j = 0; j < m->nColumns; j++)
        {
            if (m->data[i * m->nColumns + j] > max)
            {
                max = m->data[i * m->nColumns + j];
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
        (*a)[i] = 1; //RANDOM_VALUE;
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

    long n = m->nColumns;
    if (n > MAX_COLUMNS_TO_OUTPUT)
    {
        n = MAX_COLUMNS_TO_OUTPUT;
    }

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
        fprintf(fp, "\n%s\n", name);
    }

    for (long i = 0; i < m->nRows && i < n; i++)
    {
        for (long j = 0; j < m->nColumns && j < n; j++)
        {
            fprintf(fp, formatString, m->data[i * m->nColumns + j]);
        }
        if (m->nColumns > n)
        {
            fprintf(fp, " ...");
        }
        fprintf(fp, "\n");
    }
}