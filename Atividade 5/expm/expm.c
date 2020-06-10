/**
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#include "util.h"
#include "matrix.h"
#include "parse_param.h"
#include "single_process.h"
#include "multi_process.h"

int main(int argc, char *argv[])
{
    /**
     * Command line parameters
     */
    ParsedParams params;

    /**
     * Number os processes
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
     * A matrix
     */
    Matrix *a;

    /**
     * S matrix
     */
    Matrix *s;

    int res = NOK;

    //Initialize MPI environment
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("Error initializing MPI environment!\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Parse command line arguments
    params = getParams(myrank, argc, argv);

    if (myrank == 0)
    {
        // Initialize random number generation
        srand(params.seed);

        a = createMatrix(params.n, params.n);
        s = createMatrix(params.n, params.n);

        fillMatrixWithRandom(&a);

        //save A matrix to file
        printMatrixToFile(params.outputfile,
                          "A",
                          a,
                          USE_SHORT_FORMAT,
                          OVERWRITE_FILE);
    }

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0)
    {
        // Starting time
        ti = MPI_Wtime();
    }

    if (npes == 1)
    {
        //Single thread/process
        res = singleProcess(&params, a, &s);
    }
    else
    {
        res = multiProcess(&params, a, &s, myrank, npes);
    }

    if (res == OK)
    {
        // Sync everyone
        MPI_Barrier(MPI_COMM_WORLD);
        if (myrank == 0)
        {
            tf = MPI_Wtime();
            /* Elapsed time */
            printf("Elapsed time: %fs\n", tf - ti);

            printMatrixToFile(params.outputfile,
                              "S",
                              s,
                              USE_LONG_FORMAT,
                              APPEND_FILE);
        }
    }
    else
    {
        // TODO:
        // AHHH SOMETHING NOK!
    }

    if (myrank == 0)
    {
        destroyMatrix(&a);
        destroyMatrix(&s);
    }

    MPI_Finalize();
    return 0;
}
