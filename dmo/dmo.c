/**
 * 
 * Este exercício é sobre metodologia de projeto de algoritmos paralelos e
 * desenvolvimento de raciocínio “paralelo”, recorrendo a conceitos de DBPP e
 * de IPC. Pretende também ilustrar as vantagens da sobreposição de computação
 * / comunicação com o recurso às funções MPI_Isend() e MPI_Irecv()
 * 
 * Dado um ponto num espaço bidimensional (plano) com coordenadas (x,y), a sua
 * distância à origem (0,0) é dada pela fórmula d = √(x^2 + y^2) .
 * 
 * O problema a abordar e para o qual se pretende desenvolver um programa
 * paralelo para o resolver é: dado dois vetores x,y de dimensão n representando
 * coordenadas x e y, calcular a distância média à origem (dmo) de todos os n^2
 * pontos que podem ser definidos pelas combinações de coordenadas dos dois
 * vetores.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Tag to use on our messages
#define MESSAGE_TAG 1

int main(int argc, char *argv[])
{
    MPI_Status status;
    int npes, rank;
    double ti, tf, dt;

    int error = MPI_Init(&argc, &argv);
    if (error != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        // Início da contagem do tempo
        ti = MPI_Wtime();
    }

    /**
     * 1- O valor de n≥np é dado como argumento ao programa. Para facilitar o
     * programa, n é arredondado ao múltiplo de np imediatamente igual ou
     * inferior.
     * 2- O processo P0 cria os vetores x e y com uma distribuição uniforme
     * entre -2 e 2 com o auxílio da função rand() inicializada previamente
     * com a semente 223.
     * 3- P0 envia os subvetores x i ,y i (de dimensão n/np cada) para os
     * outros processos 1,...,np.
     * 4- Ciclo: Cada processo P i atualiza T i = T i + J x i y i , envia yi
     * para Pi-1 e recebe yi+1 de Pi+1
     * 5- P0 recebe os resultados parciais J x i y dos outros processos
     * 1,...,np.
     * 6- P0 calcula a soma dos resultados parciais, divide por n 2 e imprime
     * o valor final Jxy .
     */

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        tf = MPI_Wtime();
        dt = tf - ti;

        /* intervalo de tempo decorrido */

        printf("Tempo decorrido: %fs \n", dt);
    }

    MPI_Finalize();
    return 0;
}