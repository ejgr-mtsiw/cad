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
    * local A line
    */
    double *aLine;

    /**
     * Temp array for faster buffer unload
     */
    double *tmp;

    /**
     * Local A receive buffer
     */
    double *aBuffer;

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

    // Initialize random number generation
    srand(params.seed);

    if (npes == 1)
    {
        double *s = (double *)malloc(sizeof(double) * params.n * params.n);
        double *a = (double *)malloc(sizeof(double) * params.n * params.n);

        // Init A with random values
        fillWithRandom(&a, params.n);

        //Single thread/process
        int res = singleProcess(&params, a, &s);

        if (res == OK)
        {
            printMatrix("A", a, params.n, USE_SHORT_FORMAT);
            printMatrix("S", s, params.n, USE_LONG_FORMAT);

            printMatrixToFile(params.outputfile, "A", a, params.n, USE_SHORT_FORMAT, 0);
            printMatrixToFile(params.outputfile, "S", s, params.n, USE_LONG_FORMAT, 1);
        }

        free(a);
        free(s);
    }

    MPI_Finalize();
    exit(EXIT_SUCCESS);
}
