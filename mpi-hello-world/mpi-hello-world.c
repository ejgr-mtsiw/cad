#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int npes, myrank;
    
    int error = MPI_Init(&argc, &argv);
    if (error != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    
    printf("From process %d out of %d, Hello World!\n", myrank, npes);
    
    MPI_Finalize();
    return 0;
}