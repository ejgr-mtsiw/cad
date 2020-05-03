/**
 * This program calculates pi using the Monte Carlo method
 * https://en.wikipedia.org/wiki/Monte_Carlo_method
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/**
 * Number of points to generate
 */
#define NUMBER_OF_POINTS 500000000

/**
 * Minimum number of points in a workload (* WORK_LOAD_STEP)
 */
#define MIN_WORK_LOAD 1

/**
 * Maximum number of points in a workload (* WORK_LOAD_STEP)
 */
#define MAX_WORK_LOAD 10

/**
 * Step for the random workload distribution
 */
#define WORK_LOAD_STEP 1000000

// Seed for rand()
#define RAND_SEED 223

// Communication tags
/**
 * Initial slave rquest for work
 */
#define SLAVE_READY 0

/**
 * Slave returns result
 */
#define SLAVE_RESULT 1

/**
 * Master assigns new work
*/
#define SLAVE_WORK 2

/**
 * Struct to store work orders
 */
typedef struct WORKORDER
{
    int process;
    long workload;
} workorder;

int main(int argc, char *argv[])
{
    /**
     * Number of processes
     */
    int npes = 0;

    /**
     * Current process rank
     */
    int myrank = 0;

    /**
     * Timing variables
     */
    double ti, tf;

    /**
     * Number of points where x^2 + y^2 <= 1
     */
    long nPointsInside = 0;

    //Initialize MPI environment
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Minimum 2 processes
    if (npes < 2)
    {
        // Only process #0 reports the error to avoid spamming the command line
        if (myrank == 0)
        {
            printf("Minimum 2 processes\n");
        }

        MPI_Finalize();
        return 0;
    }

    // Initialize the random number generator with RAND_SEED
    srand(RAND_SEED);

    if (myrank == 0)
    {
        int workers = npes - 1;
        long workToBeAssigned = NUMBER_OF_POINTS, workAssigned = 0, workLoad = 0;
        long slaveResult = 0;
        //workorder **workLog;
        workorder workLog[(NUMBER_OF_POINTS / (MIN_WORK_LOAD * WORK_LOAD_STEP)) + 1];
        int currentWorkOrder = 0;
        MPI_Status mpiStatus;

        // Allocate memory for work log
        // Worst case scenario all rands() return 1
        //workLog = malloc(sizeof(workorder *) * ((int)(NUMBER_OF_POINTS / MIN_WORK_LOAD) + 1));

        while (workers > 0)
        {
            // Wait for incoming calls
            MPI_Recv(&slaveResult, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpiStatus);

            if (mpiStatus.MPI_TAG == SLAVE_RESULT)
            {
                //printf("Recebi %d pontos do escravo %d!\n", slaveResult, mpiStatus.MPI_SOURCE);
                nPointsInside += slaveResult;
            }

            //printf("DEBUG: workAssigned:%7d, workToBeAssigned:%7d, workToBeDone:%7d, p:%d", workAssigned, workToBeAssigned, NUMBER_OF_POINTS, mpiStatus.MPI_SOURCE);

            // Send work order
            if (workToBeAssigned == 0)
            {
                // We already assigned all the work, so this slave is dismissed
                workLoad = 0;
                workers--;
            }
            else
            {
                // Generate random workload without going over the limit
                workLoad = (rand() % (MAX_WORK_LOAD - MIN_WORK_LOAD) + MIN_WORK_LOAD) * WORK_LOAD_STEP;
                if (workLoad > workToBeAssigned)
                {
                    workLoad = workToBeAssigned;
                }

                workToBeAssigned -= workLoad;
                workAssigned += workLoad;

                // Log work order
                // workLog[currentWorkOrder] = malloc(sizeof(workorder));
                // workLog[currentWorkOrder]->process = mpiStatus.MPI_SOURCE;
                // workLog[currentWorkOrder]->workload = workLoad;
                workLog[currentWorkOrder].process = mpiStatus.MPI_SOURCE;
                workLog[currentWorkOrder].workload = workLoad;
                currentWorkOrder++;
            }

            //printf(", workLoad: %7d, workers:%d\n", workLoad, workers);

            MPI_Send(&workLoad, 1, MPI_LONG, mpiStatus.MPI_SOURCE, SLAVE_WORK, MPI_COMM_WORLD);
        }

        //printf("nPointsInside: %d\n", nPointsInside);
        printf("Estimativa de pi = %.8f\n", (double)nPointsInside / NUMBER_OF_POINTS * 4);

        printf("Relatório trabalho total atribuído [%d]\n", NUMBER_OF_POINTS);

        for (int i = 0; i < currentWorkOrder; i++)
        {
            printf("Escravo %d atribuído trabalho %d\n", workLog[i].process, workLog[i].workload);
            //printf("Escravo %d atribuído trabalho %d\n", workLog[i]->process, workLog[i]->workload);
            //free(workLog[i]);
        }

        //free(workLog);
    }
    else
    {
        /**
         * Number of points to process
         */
        long nPointsToProcess = 0;

        /**
         * Random x and y
         */
        double rX, rY;

        // Request first work order
        MPI_Send(NULL, 0, MPI_LONG, 0, SLAVE_READY, MPI_COMM_WORLD);

        for (;;)
        {
            //printf("Escravo %d à espera de trabalho!\n", myrank);
            // Wait for workload info
            MPI_Recv(&nPointsToProcess, 1, MPI_LONG, 0, SLAVE_WORK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            //printf("Escravo %d recebeu %d trabalho! ", myrank, nPointsToProcess);

            if (nPointsToProcess <= 0)
            {
                // No more work for us -> exit
                break;
            }

            nPointsInside = 0;

            for (int j = 0; j < nPointsToProcess; j++)
            {
                rX = (double)rand() / RAND_MAX;
                rY = (double)rand() / RAND_MAX;

                if ((rX * rX + rY * rY) <= 1)
                {
                    nPointsInside++;
                }
            }

            //printf("Escravo %d enviou %d pontos dentro!\n", myrank, nPointsInside);
            MPI_Send(&nPointsInside, 1, MPI_LONG, 0, SLAVE_RESULT, MPI_COMM_WORLD);
        }

        printf("Escravo %d terminou!\n", myrank);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
