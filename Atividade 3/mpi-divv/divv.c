/**
 * This program shares an array between all the processes in the pool using
 * blocks of similar size using  MPI_Scatterv
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// First element controlled by process id out of p processes, array length n
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))

// Last element controlled by process id out of p processes, array length n
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id + 1), p, n) - 1)

// Size of the block controlled by process id out of p processes, array length n
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id + 1), p, n) - BLOCK_LOW(id, p, n))

// Process that controls item index from array with length n, p processes
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + 1) - 1) / (n))

int main(int argc, char *argv[])
{
    int npes = 0, myrank = 0, n = 0;
    int *data, nItemsToProcess = 0, *sendcounts, *displs;

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

    // Number of numbers to process must be >= 3 * npes
    if (n < 3 * npes)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
        {
            printf("Usage: %s ARRAY_LENGTH\nARRAY_LENGTH >= 3 * N_PROCESSES\n",
                   argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    nItemsToProcess = BLOCK_SIZE(myrank, npes, n);

    if (myrank == 0)
    {
        data = malloc(sizeof(int) * n);

        // Process #0 initializes the array to be shared
        for (int i = 0; i < n; i++)
        {
            data[i] = i;
        }

        // Allocate sendcounts array
        sendcounts = malloc(sizeof(int) * npes);

        // Allocate displacements array
        displs = malloc(sizeof(int) * npes);

        for (int i = 0; i < npes; i++)
        {
            sendcounts[i] = BLOCK_SIZE(i, npes, n);
            displs[i] = BLOCK_LOW(i, npes, n);
        }

        // Scatter the array by the processes using the distribution set by the
        // sendcounts and displs arrays. Root uses MPI_IN_PLACE so no data
        // is ever sent, saving one send/receive
        // https://www.open-mpi.org/doc/v4.0/man3/MPI_Scatterv.3.php#toc8
        MPI_Scatterv(data,
                     sendcounts,
                     displs,
                     MPI_INT,
                     MPI_IN_PLACE,
                     nItemsToProcess,
                     MPI_INT,
                     0,
                     MPI_COMM_WORLD);
    }
    else
    {
        // Allocate receive buffer
        // Caso existam elementos com 0 items para processar:
        // Upon successful completion with size not equal to 0, malloc() shall
        // return a pointer to the allocated space. If size is 0, either a null
        // pointer or a unique pointer that can be successfully passed to
        // free() shall be returned.
        // https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html
        data = malloc(sizeof(int) * nItemsToProcess);

        // Receive the scattered data
        // All arguments to the function are significant on process root,
        // while on other processes, only arguments recvbuf, recvcount,
        // recvtype, root, comm are significant.
        MPI_Scatterv(data,
                     sendcounts,
                     displs,
                     MPI_INT,
                     data,
                     nItemsToProcess,
                     MPI_INT,
                     0,
                     MPI_COMM_WORLD);
    }

    // Print (part of) the array
    printf("Processo %d: v[%d] = %d, %d, ... , %d\n",
           myrank,
           nItemsToProcess,
           data[0],
           data[1],
           data[nItemsToProcess - 1]);

    // Free memory
    if (myrank == 0)
    {
        free(sendcounts);
        free(displs);
    }

    free(data);

    MPI_Finalize();
    return 0;
}
