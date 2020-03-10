/**
 * This program calculates the sum of the running processes spawned
 * Each process will add its value (rank + 1) to the running total
 * and pass it to the next process, starting with process 0.
 * The last process sends it back to process 0 that prints the total.
 * 
 * Ex: mpirun -np 5 somanp
 * processo 0: enviado 1
 * processo 1: recebido 1
 * processo 1: enviado 3
 * processo 2: recebido 3
 * processo 2: enviado 6
 * processo 3: recebido 6
 * processo 4: recebido 10
 * processo 3: enviado 10
 * processo 4: enviado 15
 * processo 0: recebido 15
 * Total: 15
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Tag to use on our messages
#define MESSAGE_TAG 1

int main(int argc, char *argv[])
{
    int npes, myrank;
    int np = 0, destination = 0;
    MPI_Status status;

    int erro = MPI_Init(&argc, &argv);
    if (erro != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (npes == 1)
    {
        // Only one process
        // Just print 0 and exit
        printf("Only one process...\nTotal: 1\n");

        MPI_Finalize();
        return 0;
    }

    if (myrank != 0)
    {
        // Not first process
        // Listen for a message from the previous process
        MPI_Recv(&np, 1, MPI_INT, myrank - 1, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        printf("processo %d: recebido %d\n", myrank, np);
    }

    // Add our rank plus one
    np += myrank + 1;

    if (npes == myrank + 1)
    {
        // We're the last process
        // Send total to process 0
        destination = 0;
    }
    else
    {
        // pass it along
        destination = myrank + 1;
    }

    // send message
    MPI_Send(&np, 1, MPI_INT, destination, MESSAGE_TAG, MPI_COMM_WORLD);
    printf("processo %d: enviado %d\n", myrank, np);

    if (myrank == 0)
    {
        // First process
        // Listen for the message from the last process
        MPI_Recv(&np, 1, MPI_INT, npes - 1, MESSAGE_TAG, MPI_COMM_WORLD, &status);

        printf("processo %d: recebido %d\n", myrank, np);
        printf("Total: %d\n", np);
    }

    MPI_Finalize();
    return 0;
}