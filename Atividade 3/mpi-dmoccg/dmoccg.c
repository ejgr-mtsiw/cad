/**
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
 * Escreva o programa dmoccg.c em linguagem C/MPI semelhante ao programa
 * dmocc.c do exercício S2.4 mas utilizando as operações de comunicação
 * globais Scatter e Reduce.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// Seed for rand()
#define RAND_SEED 223

// Max for rand()
#define MAX_RAND_VALUE 2

// Min for rand()
#define MIN_RAND_VALUE -2

/**
 * Calculte random workload based on the #define's above
 */
#define RANDOM_VALUE ((double)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE)

/**
 * Update Ti
 */
double calculateDistanceToOrigin(double *x, double *y, int dataLength)
{
    double localTotal = 0.0;
    for (int i = 0; i < dataLength; i++)
    {
        for (int j = 0; j < dataLength; j++)
        {
            localTotal += sqrt(x[i] * x[i] + y[j] * y[j]);
        }
    }

    return localTotal;
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
     * Dimension of the main array
     */
    int n = 0;

    /**
    * x array to be shared
    */
    double *xToSend;

    /**
    * y array to be shared
    */
    double *yToSend;

    /**
    * local x array
    */
    double *x;

    /**
    * local y array
    */
    double *y;

    /**
     * Temp array for faster buffer unload
     */
    double *tmp;

    /**
     * Local y receive buffer
     */
    double *yBuffer;

    /**
    * Send counts
    */
    int *sendcounts = NULL;

    /**
    * Displacements
    */
    int *displs = NULL;

    /**
     * Local total
     */
    double localTotal = 0.0;

    /**
     * Global total
     */
    double globalTotal = 0.0;

    /**
     * Timing variables
     */
    double ti, tf;

    /**
     * MPI_Requests to control delivery
     */
    MPI_Request xRequest, yRequest;

    //Initialize MPI environment
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Check input arguments
    if (argc < 2)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
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
        if (myrank == 0)
        {
            printf("USAGE: %s ARRAY_LENGTH\n", argv[0]);
            printf("ARRAY_LENGTH must be higher than process count\n");
        }

        MPI_Finalize();
        return 0;
    }

    if (n % npes != 0)
    {
        n = ((int)(n / npes)) * npes;
    }

    // Length of the array to send to each process
    int dataLength = n / npes;

    // Allocate buffers
    x = malloc(dataLength * sizeof(double));
    y = malloc(dataLength * sizeof(double));
    yBuffer = malloc(dataLength * sizeof(double));

    /*
    2- O processo P0 cria os vetores x e y com uma distribuição uniforme
    entre -2 e 2 com o auxílio da função rand() inicializada previamente
    com a semente 223.
    */
    if (myrank == 0)
    {
        // allocate memory
        xToSend = malloc(n * sizeof(double));
        yToSend = malloc(n * sizeof(double));

        // Initialize the random number generator with SEED
        srand(RAND_SEED);

        // Fill both arrays with values between MIN_RAND_VALUE and MAX_RAND_VALUE
        for (int i = 0; i < n; i++)
        {
            xToSend[i] = RANDOM_VALUE;
            yToSend[i] = RANDOM_VALUE;
        }

        // Allocate sendcounts array
        sendcounts = malloc(npes * sizeof(int));

        for (int i = 0; i < npes; i++)
        {
            sendcounts[i] = dataLength;
        }

        // Allocate displacements array
        // By allocating twice as many elements we can use the array
        // as a sliding window in MPI_Iscatterv
        displs = malloc(2 * npes * sizeof(int));

        for (int i = 0; i < 2 * npes; i++)
        {
            displs[i] = (i % npes) * dataLength;
        }
    }

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0)
    {
        // Starting time
        ti = MPI_Wtime();
    }

    // Scatter the x array by the processes using the distribution set
    // by the sendcounts and displs arrays
    MPI_Scatterv(xToSend,
                 sendcounts,
                 displs,
                 MPI_DOUBLE,
                 x,
                 dataLength,
                 MPI_DOUBLE,
                 0,
                 MPI_COMM_WORLD);

    /*
     4- Ciclo: Cada processo Pi atualiza Ti = Ti + Jxiyi , envia yi
     para Pi-1 e recebe yi+1 de Pi+1
    */
    for (int cycle = 0; cycle < npes; cycle++)
    {
        // Scatter y
        MPI_Iscatterv(yToSend,
                      sendcounts,
                      &displs[cycle],
                      MPI_DOUBLE,
                      yBuffer,
                      dataLength,
                      MPI_DOUBLE,
                      0,
                      MPI_COMM_WORLD,
                      &yRequest);

        // On the first cycle we don't have data to work so skip it
        if (cycle > 0)
        {
            // Update Ti
            localTotal += calculateDistanceToOrigin(x, y, dataLength);
        }

        // making sure we have the data we need to work
        MPI_Wait(&yRequest, MPI_STATUS_IGNORE);

        tmp = y;
        y = yBuffer;
        yBuffer = tmp;
    }

    // Update Ti (last batch)
    localTotal += calculateDistanceToOrigin(x, y, dataLength);

    /*
     5- P0 recebe os resultados parciais Jxiy dos outros processos 1,...,np.
    */
    MPI_Reduce(&localTotal, &globalTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0)
    {
        tf = MPI_Wtime();
        /* Elapsed time */
        //printf("Elapsed time on task #3 ~ #6: %fs\n", tf - ti);
        printf("%f\n", tf - ti);
        // Calculate average
        double jxy = globalTotal / (n * n);
        //printf("A distância média dos elementos à origem é %.2f\n", jxy);
    }

    // Free allocated memory
    free(x);
    free(y);
    free(yBuffer);

    if (myrank == 0)
    {
        free(yToSend);
        free(xToSend);

        free(displs);
        free(sendcounts);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
