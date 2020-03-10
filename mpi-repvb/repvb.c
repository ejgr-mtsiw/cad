/**
 * This program shares an array between all the processes in the pool
 * 
 * Process #0 creates an array with the length indicated by the user in the
 * command line.
 * 
 * In each iteration of the algorithm all the processes with rank k < 2^i send
 * the array to the process 2^i + k.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Tag to use on our messages
#define MESSAGE_TAG 1

int main(int argc, char *argv[])
{
    int npes, myrank;
    int currentPower = 1, n = 0;
    int *data;
    MPI_Status status;

    int error = MPI_Init(&argc, &argv);
    if (error != MPI_SUCCESS)
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

    if (myrank == 0)
    {
        // Process #0 initializes the data array
        data = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++)
        {
            data[i] = i;
        }
    }

    for (int i = 0; currentPower < npes; i++)
    {
        // If myrank < 2^i and there's a suitable destination, send the array
        if (myrank < currentPower && npes > myrank + currentPower)
        {
            //printf("[%d] #%d sending to #%d\n", i, myrank, myrank + currentPower);
            MPI_Send(data, n, MPI_INT, myrank + currentPower, MESSAGE_TAG, MPI_COMM_WORLD);
        }
        else
        {
            // if 2^i <= myrank < 2^(i+1) listen to a message with the array
            if (currentPower <= myrank && myrank < currentPower * 2)
            {
                //printf("[%d] #%d receiving from #%d\n", i, myrank, myrank - currentPower);
                MPI_Recv(data, n, MPI_INT, myrank - currentPower, MESSAGE_TAG, MPI_COMM_WORLD, &status);
            }
        }

        currentPower *= 2;
    }

    // Print (part of) the array
    printf("processo %d: v[%d] = %d, %d, ..., %d\n", myrank, n, data[0], data[1], data[n - 1]);

    // Only process #0 needs to free the data array, because it was allocated
    // using malloc.
    // The MPI implementation will take care of it for the other processes
    if (myrank == 0)
    {
        free(data);
    }

    MPI_Finalize();
    return 0;
}