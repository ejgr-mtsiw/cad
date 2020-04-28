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

/**
 * 1- O valor de n≥np é dado como argumento ao programa. Para facilitar o
 * programa, n é arredondado ao múltiplo de np imediatamente igual ou
 * inferior.
 * 2- O processo P0 cria os vetores x e y com uma distribuição uniforme
 * entre -2 e 2 com o auxílio da função rand() inicializada previamente
 * com a semente 223.
 * 3- P0 envia os subvetores xi ,yi (de dimensão n/np cada) para os
 * outros processos 1,...,np.
 * 4- Ciclo: Cada processo Pi atualiza Ti = Ti + Jxiyi , envia yi
 * para Pi-1 e recebe yi+1 de Pi+1
 * 5- P0 recebe os resultados parciais Jxiy dos outros processos
 * 1,...,np.
 * 6- P0 calcula a soma dos resultados parciais, divide por n^2 e imprime
 * o valor final Jxy.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// Tag to use on our messages
#define MESSAGE_TAG_INITIAL_X 1
#define MESSAGE_TAG_INITIAL_Y 2
#define MESSAGE_TAG_Y_LINE 3
#define MESSAGE_TAG_SUB_TOTAL 4

// Seed for rand()
#define RAND_SEED 223

// Max for rand()
#define MAX_RAND_VALUE 2

// Min for rand()
#define MIN_RAND_VALUE -2

/**
 * Gets next process rank
 */
int getNextProcessRank(int rank, int npes)
{
    return (rank + 1) % npes;
}

/**
 * Gets previuos process rank
 */
int getPreviousProcessRank(int rank, int npes)
{
    return (npes + rank - 1) % npes;
}

int main(int argc, char *argv[])
{
    MPI_Request sendRequest, getRequest;
    int npes = 0, rank = 0, originTarget = 0, destinationTarget = 0;
    int i, j;

    /**
     * Timing variables
     */
    double ti, tf;

    /**
     * Length of each array to send/receive
     */
    int dataLength = 0;

    /**
    * Data arrays
    */
    float *y, *x, *yToReceive;

    /**
     * Current total
     */
    double Ti = 0.0;

    /**
     * Dimension of the arrays
     */
    int n = 0;

    //Initizlize MPI environment
    int error = MPI_Init(&argc, &argv);
    if (error != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Check input arguments
    if (argc < 2)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (rank == 0)
        {
            printf("USAGE: %s ARRAY_LENGTH\n", argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    /*
     1- O valor de n≥np é dado como argumento ao programa. Para facilitar o
     programa, n é arredondado ao múltiplo de np imediatamente igual ou
     inferior.
    */
    n = atoi(argv[1]);

    if (n < npes)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (rank == 0)
        {
            printf("USAGE: %s ARRAY_LENGTH\n", argv[0]);
            printf("ARRAY_LENGTH must be higher than process count");
        }

        MPI_Finalize();
        return 0;
    }

    if (n % npes != 0)
    {
        n = ((int)(n / npes)) * npes;
    }

    // Length of the array to send to each process
    dataLength = n / npes;

    // Buffer for the y array to receive so we don't overwrite our data
    yToReceive = malloc(dataLength * sizeof(float));

    // The process that will send me the next y[]
    originTarget = getNextProcessRank(rank, npes);

    // Process that will receive my y[]
    destinationTarget = getPreviousProcessRank(rank, npes);

    /*
    2- O processo P0 cria os vetores x e y com uma distribuição uniforme
    entre -2 e 2 com o auxílio da função rand() inicializada previamente
    com a semente 223.
    */
    if (rank == 0)
    {
        // allocate memory
        x = malloc(n * sizeof(float));
        y = malloc(n * sizeof(float));

        // Initialize the random number generator with SEED
        srand(RAND_SEED);

        // Fill both arrays with values between MIN_RAND_VALUE and MAX_RAND_VALUE
        for (i = 0; i < n; i++)
        {
            x[i] = (float)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE;
            y[i] = (float)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE;
        }
    }

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        // Starting time
        ti = MPI_Wtime();
    }

    /*
     3- P0 envia os subvetores xi ,yi (de dimensão n/np cada) para os
     outros processos 1,...,np.
    */
    if (rank == 0)
    {
        // Send partial data to each process
        for (i = 1; i < npes; i++)
        {
            MPI_Send(&x[i * dataLength], dataLength, MPI_FLOAT, i, MESSAGE_TAG_INITIAL_X, MPI_COMM_WORLD);
            MPI_Send(&y[i * dataLength], dataLength, MPI_FLOAT, i, MESSAGE_TAG_INITIAL_Y, MPI_COMM_WORLD);
        }
    }
    else
    {
        // All processes except 0 can use smaller arrays
        x = malloc(dataLength * sizeof(float));
        y = malloc(dataLength * sizeof(float));

        // Receive the x and y data for this process
        MPI_Recv(x, dataLength, MPI_FLOAT, 0, MESSAGE_TAG_INITIAL_X, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(y, dataLength, MPI_FLOAT, 0, MESSAGE_TAG_INITIAL_Y, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    /*
     4- Ciclo: Cada processo Pi atualiza Ti = Ti + Jxiyi , envia yi
     para Pi-1 e recebe yi+1 de Pi+1
    */
    for (int cycle = 0; cycle < npes; cycle++)
    {
        // We already have our own data sent from #0 se we only need to
        // send/recieve npes - 1 times
        if (cycle < npes - 1)
        {
            // Trying to improve performance: As MPI_Isend doesn't block we'll
            // send the data we have while we're working on the calculations
            // Using Isend and Irecv we no longer need to impose a sending order
            // Do Irecv before Isend:
            // http://supercomputingblog.com/mpi/mpi-tutorial-5-asynchronous-communication/
            MPI_Irecv(yToReceive, dataLength, MPI_FLOAT, originTarget, MESSAGE_TAG_Y_LINE, MPI_COMM_WORLD, &getRequest);
            MPI_Isend(y, dataLength, MPI_FLOAT, destinationTarget, MESSAGE_TAG_Y_LINE, MPI_COMM_WORLD, &sendRequest);
        }

        // Update Ti
        for (i = 0; i < dataLength; i++)
        {
            for (j = 0; j < dataLength; j++)
            {
                Ti += sqrt(x[i] * x[i] + y[j] * y[j]);
            }
        }

        // We already have our own data sent from #0 se we only need to
        // send/recieve npes - 1 times
        if (cycle < npes - 1)
        {
            // we will change the data in the send buffer so we need to make
            // sure it's safe to proceed
            MPI_Wait(&sendRequest, MPI_STATUS_IGNORE);

            // making sure we have the data we need to work
            MPI_Wait(&getRequest, MPI_STATUS_IGNORE);

            for (i = 0; i < dataLength; i++)
            {
                // Fill the y values from the receive buffer
                y[i] = yToReceive[i];
            }
        }
    }

    /*
     5- P0 recebe os resultados parciais Jxiy dos outros processos 1,...,np.
    */
    if (rank == 0)
    {
        // Buffer to colect partial results from the other processes
        double jxiy = 0.0;

        for (i = 1; i < npes; i++)
        {
            MPI_Recv(&jxiy, 1, MPI_DOUBLE, i, MESSAGE_TAG_SUB_TOTAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            Ti += jxiy;
        }
    }
    else
    {
        // Send our partial sum to rank 0
        MPI_Send(&Ti, 1, MPI_DOUBLE, 0, MESSAGE_TAG_SUB_TOTAL, MPI_COMM_WORLD);
    }

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        tf = MPI_Wtime();
        /* Elapsed time */
        printf("Elapsed time on task #3 ~ #6: %fs\n", tf - ti);

        // Calculate average
        double jxy = Ti / (n * n);
        printf("A distância média dos elementos à origem é %.2f\n", jxy);
    }

    // Free allocated memory
    free(x);
    free(y);
    free(yToReceive);

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
