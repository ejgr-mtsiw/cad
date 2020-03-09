#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char *argv[])
{
    int erro = MPI_Init(&argc, &argv);

    if (erro == MPI_SUCCESS)
    {
        printf("Ambiente MPI inicializado com sucesso!\n");
        MPI_Finalize();
    } else {
        printf("Erro na inicialização do ambiente MPI!\n");
        exit(1);
    }

    return 0;
}

// mpicc -o teste teste-mpi.c
// mpirun -np 2 teste

// EOF