#include "matrix.h"

double maxMij(const double *m, long n)
{
    double max = 0.0;

    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            if (m[i * n + j] > max)
            {
                max = m[i * n + j];
            }
        }
    }

    return max;
}

int fillWithRandom(double **a, long n)
{
    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            (*a)[i * n + j] = RANDOM_VALUE;
        }
    }
    return OK;
}

int sumMatrix(const double *m, double **s, long n)
{
    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            (*s)[i * n + j] += m[i * n + j];
        }
    }

    return OK;
}

int setIdentityMatrix(double **s, long n)
{
    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            if (i == j)
            {
                (*s)[i * n + j] = 1.0;
            }
            else
            {
                (*s)[i * n + j] = 0.0;
            }
        }
    }

    return OK;
}

int multiplyMatrix(const double *a, const double *b, double **multiplied, long n)
{
    double val;

    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            val = 0.0;
            for (long k = 0; k < n; k++)
            {
                val += a[i * n + j] * b[i * k + j];
            }

            (*multiplied)[i * n + j] = val;
        }
    }

    return OK;
}

int divideMatrixByLong(double **a, long number, long n)
{
    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            (*a)[i * n + j] = (*a)[i * n + j] / number;
        }
    }

    return OK;
}

void printMatrix(const char *name, const double *m, long n, int format)
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
        printf("\n%s\n", name);
    }

    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            printf(formatString, m[i * n + j]);
        }
        printf("\n");
    }
}

int printMatrixToFile(const char *filename, const char *name, const double *m, long n, int format, int append)
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

    if (n > 20)
    {
        n = 20;
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
        fprintf(outputFile, "\n%s\n", name);
    }

    for (long i = 0; i < n; i++)
    {
        for (long j = 0; j < n; j++)
        {
            fprintf(outputFile, formatString, m[i * n + j]);
        }
        fprintf(outputFile, "\n");
    }

    fclose(outputFile);

    return OK;
}
