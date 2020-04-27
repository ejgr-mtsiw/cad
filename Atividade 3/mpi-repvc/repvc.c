/**
 * This program shares an array between all the processes in the pool
 * using MPI_Bcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int npes = 0, myrank = 0, n = 0;
    int *data;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (argc < 2)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
        {
            printf("Usage: %s ARRAY_LENGTH\n", argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    n = atoi(argv[1]);

    if (n < 3)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
        {
            printf("Usage: %s ARRAY_LENGTH\nARRAY_LENGTH >= 3\n", argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    data = malloc(sizeof(int) * n);

    if (myrank == 0)
    {
        // Process #0 initializes the data array
        for (int i = 0; i < n; i++)
        {
            data[i] = i;
        }
    }

    MPI_Bcast(data, n, MPI_INT, 0, MPI_COMM_WORLD);

    // Print (part of) the array
    printf("Processo %d: v[%d] = %d, %d, ..., %d\n",
           myrank,
           n,
           data[0],
           data[1],
           data[n - 1]);

    free(data);

    MPI_Finalize();
    return 0;
}
