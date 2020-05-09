/**
 * This program calculates pi using the Monte Carlo method
 * https://en.wikipedia.org/wiki/Monte_Carlo_method
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <sys/time.h>
#include <stdbool.h>

/**
 * Number of points to generate
 */
#define N_POINTS 500000000L

/**
 * Minimum number of points in a workload (* WORKLOAD_STEP)
 */
#define MIN_WORKLOAD 1

/**
 * Maximum number of points in a workload (* WORKLOAD_STEP)
 */
#define MAX_WORKLOAD 10

/**
 * Step for the random workload distribution
 */
#define WORKLOAD_STEP 1000000

/**
 * Calculte random workload based on the #define's above
 */
#define RANDOM_WORKLOAD ((MIN_WORKLOAD + rand() % (MAX_WORKLOAD - MIN_WORKLOAD + 1)) * WORKLOAD_STEP)

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
typedef struct workorder
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
     * Number of points where x^2 + y^2 <= 1
     */
    long nPointsIn = 0;

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

    // Initialize the random number generator with a random seed
    // We're using useconds because the processes are started in rapid
    // succession so we need a subsecond seed generator to avoid repeating seeds
    struct timeval time;
    gettimeofday(&time, NULL);
    srand(time.tv_usec);

    if (myrank == 0)
    {
        /**
         * Number of slaves
         */
        int slaves = npes - 1;

        /**
         * Remaining work to be done
         */
        long workToBeAssigned = N_POINTS;

        /**
         * Workload
         */
        long workload = 0;

        /**
         * Result from slave process
         */
        long slaveResult = 0;

        /**
         * Work log - We're allocating the full array (worst case scenario)
         * to avoid having to do dynamic allocation
         * Best case scenario for 500M points in 1M to 10M workloads:
         * array_length: 50 -> wasted 450*12 bytes, avoided 50 malloc()
         * Worst case:
         * array_length: 500 -> wasted 0 bytes, avoided 500 malloc()
         */
        workorder workLog[(N_POINTS / (MIN_WORKLOAD * WORKLOAD_STEP)) + 1];

        /**
         * Work log index
         */
        int currentWorkOrder = 0;

        /**
         * MPI_Status used to differentiate between request types
         */
        MPI_Status mpiStatus;

        /**
         * MPI_Request to use MPI_Isend
         */
        MPI_Request mpiRequest;

        /**
         * After the first send we need to MPI_Wait before changing workload
         */
        bool firstSend = true;

        while (slaves > 0)
        {
            // Wait for incoming calls
            MPI_Recv(&slaveResult,
                     1,
                     MPI_LONG,
                     MPI_ANY_SOURCE,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &mpiStatus);

            // We can receive 2 types of calls:
            // SLAVE_READY when the slave is ready to start working
            // SLAVE_RESULT if it has data to be processed
            if (mpiStatus.MPI_TAG == SLAVE_RESULT)
            {
                nPointsIn += slaveResult;
            }

            // Making sure we can change workLoad
            if (firstSend)
            {
                firstSend = false;
            }
            else
            {
                MPI_Wait(&mpiRequest, MPI_STATUS_IGNORE);
            }

            // Send work order
            if (workToBeAssigned == 0)
            {
                // We already assigned all the work, so this slave is dismissed
                workload = 0;
                slaves--;
            }
            else
            {
                // Generate random workload without going over the limit
                workload = RANDOM_WORKLOAD;
                if (workload > workToBeAssigned)
                {
                    workload = workToBeAssigned;
                }

                workToBeAssigned -= workload;

                // Log work order
                workLog[currentWorkOrder].process = mpiStatus.MPI_SOURCE;
                workLog[currentWorkOrder].workload = workload;
                currentWorkOrder++;
            }

            MPI_Isend(&workload,
                      1,
                      MPI_LONG,
                      mpiStatus.MPI_SOURCE,
                      SLAVE_WORK,
                      MPI_COMM_WORLD,
                      &mpiRequest);
        }

        printf("Estimativa de pi = %.8f\n", (double)nPointsIn / N_POINTS * 4);

        printf("Relatório do trabalho total atribuído [%ld]\n", N_POINTS);
        for (int i = 0; i < currentWorkOrder; i++)
        {
            printf("Escravo %d atribuído trabalho %ld\n",
                   workLog[i].process,
                   workLog[i].workload);
        }
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
        MPI_Sendrecv(NULL,
                     0,
                     MPI_LONG,
                     0,
                     SLAVE_READY,
                     &nPointsToProcess,
                     1,
                     MPI_LONG,
                     0,
                     SLAVE_WORK,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

        while (nPointsToProcess > 0)
        {
            nPointsIn = 0;

            for (int j = 0; j < nPointsToProcess; j++)
            {
                rX = (double)rand() / RAND_MAX;
                rY = (double)rand() / RAND_MAX;

                if ((rX * rX + rY * rY) <= 1.0)
                {
                    nPointsIn++;
                }
            }

            MPI_Sendrecv(&nPointsIn,
                         1,
                         MPI_LONG,
                         0,
                         SLAVE_RESULT,
                         &nPointsToProcess,
                         1,
                         MPI_LONG,
                         0,
                         SLAVE_WORK,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
        }

        printf("Escravo %d terminou!\n", myrank);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
