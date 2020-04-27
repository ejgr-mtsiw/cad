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
    int *sendbuf, *recvbuf, *sendcounts, *displs;

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

    if (myrank == 0)
    {
        sendbuf = malloc(sizeof(int) * n);

        // Process #0 initializes the array to be shared
        for (int i = 0; i < n; i++)
        {
            sendbuf[i] = i;
        }
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

    // Allocate receive buffer
    recvbuf = malloc(sizeof(int) * sendcounts[myrank]);

    // Scatter the array by the processes using the distribution set by the
    // sendcounts and displs arrays
    MPI_Scatterv(sendbuf,
                 sendcounts,
                 displs,
                 MPI_INT,
                 recvbuf,
                 sendcounts[myrank],
                 MPI_INT,
                 0,
                 MPI_COMM_WORLD);

    // Print (part of) the array
    printf("Processo %d: v[%d] = %d, %d, ... , %d\n",
           myrank,
           sendcounts[myrank],
           recvbuf[0],
           recvbuf[1],
           recvbuf[sendcounts[myrank] - 1]);

    // Free memory
    if (myrank == 0)
    {
        free(sendbuf);
    }

    free(sendcounts);
    free(displs);
    free(recvbuf);

    MPI_Finalize();
    return 0;
}
