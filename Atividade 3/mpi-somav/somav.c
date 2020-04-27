/**
 * (i) O processo P0 cria o vetor com uma distribuição uniforme entre -1 e 2
 * com o auxílio da função rand() inicializada previamente com a semente 551,
 * cuja dimensão é dada como argumento na linha de comandos.
 * (ii) Após a divisão e distribuição do vetor, cada processo deve calcular a
 * soma parcial correspondente e o processo 0 deve calcular a soma total a
 * partir das parciais utilizando a operação de Reduce MPI_Reduce.
 * Cada processo deve imprimir a mensagem “processo X: soma parcial=X” e
 * adicionalmente o processo 0 deve imprimir “processo X: soma total= X”.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// Seed for rand()
#define RAND_SEED 551

// Max for rand()
#define MAX_RAND_VALUE 2

// Min for rand()
#define MIN_RAND_VALUE -1

// First element controlled by process id out of p processes, array length n
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))

// Last element controlled by process id out of p processes, array length n
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id + 1), p, n) - 1)

// Size of the block controlled by process id out of p processes, array length n
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id + 1), p, n) - BLOCK_LOW(id, p, n))

// Process that controls item index from array with length n, p processes
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + 1) - 1) / (n))

/**
 * Prints the array line with debug info
 */
void printArrayLine(int rank, int n, float *data)
{
    switch (n)
    {
    case 0:
        printf("Processo %d: nada para processar...\n", rank);
        break;
    case 1:
        printf("Processo %d: v[%d] = %.2f\n",
               rank,
               n,
               data[0]);
        break;
    case 2:
        printf("Processo %d: v[%d] = %.2f, %.2f\n",
               rank,
               n,
               data[0],
               data[1]);
        break;
    default:
        printf("Processo %d: v[%d] = %.2f, %.2f, ... , %.2f\n",
               rank,
               n,
               data[0],
               data[1],
               data[n - 1]);
        break;
    }
}

int main(int argc, char *argv[])
{
    /**
     * Number os processes
     */
    int npes = 0;

    /**
     * Current process rank
     */
    int myrank = 0;

    /**
     * Length of the main array
     */
    int n = 0;

    /**
    * Array to sum
    */
    float *data;

    /**
    * Receive buffer
    */
    float *recvbuf;

    /**
    * Send counts
    */
    int *sendcounts;

    /**
    * Displacemente
    */
    int *displs;

    /**
     * Current total
     */
    float ti = 0.0;

    /**
     * Array total
     */
    float total = 0.0;

    //Initialize MPI environment
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Check input arguments
    if (argc < 2 || atoi(argv[1]) < 1)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
        {
            printf("USAGE: %s ARRAY_LENGTH\n", argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    n = atoi(argv[1]);

    if (myrank == 0)
    {
        // allocate memory
        data = malloc(sizeof(float) * n);

        // Initialize the random number generator with SEED
        srand(RAND_SEED);

        // Fill both arrays with values between MIN_RAND_VALUE and MAX_RAND_VALUE
        for (int i = 0; i < n; i++)
        {
            data[i] = (float)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE;
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
    recvbuf = malloc(sizeof(float) * sendcounts[myrank]);

    // Scatter the array by the processes using the distribution set by the
    // sendcounts and displs arrays
    MPI_Scatterv(data,
                 sendcounts,
                 displs,
                 MPI_FLOAT,
                 recvbuf,
                 sendcounts[myrank],
                 MPI_FLOAT,
                 0,
                 MPI_COMM_WORLD);

    // Print debug line
    printArrayLine(myrank, sendcounts[myrank], recvbuf);

    // Calculate local sum
    for (int i = 0; i < sendcounts[myrank]; i++)
    {
        ti += recvbuf[i];
    }

    printf("Processo %d: soma parcial = %.2f\n", myrank, ti);

    // Reduce all sums and store result in total
    MPI_Reduce(&ti, &total, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (myrank == 0)
    {
        printf("Processo %d: ***************************** soma total = %.2f\n", myrank, total);
        free(data);
    }

    free(sendcounts);
    free(displs);
    free(recvbuf);

    MPI_Finalize();
    return 0;
}
