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
    * Send counts
    */
    int *sendcounts = NULL;

    /**
    * Displacements
    */
    int *displs = NULL;

    /**
    * local A line
    */
    Matrix *a;

    /**
     * Local S line
     */
    Matrix *s;

    /**
     * Used by the root process to distribute the A matrix
     */
    double *aToSend;

    /**
     * Temp array for faster buffer unload
     */
    double *tmp;

    /**
     * Local receive buffer
     */
    double *recvBuffer;

    // Number of rows for each process
    long nRowsPerProcess = 0;

    // Number of columns shortcut
    long nColumnsPerProcess = 0;

    // Number os items sent to each process
    long dataLength = 0;

    int res = NOK;

    /**
     * MPI_Requests to control delivery
     */
    MPI_Request mSendRequest, mRecvRequest;

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

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0)
    {
        // Starting time
        ti = MPI_Wtime();
    }

    if (npes == 1)
    {
        s = createMatrix(params.n, params.n);
        a = createMatrix(params.n, params.n);

        // Init A with random values
        fillMatrixWithRandom(&a);

        //Single thread/process
        res = singleProcess(&params, a, &s);
    }
    else
    {
        /*
            OPÇÃO 1:
         1- O valor de n≥np é dado como argumento ao programa. Para facilitar o
         programa, n é arredondado ao múltiplo de np imediatamente igual ou
         inferior.
        */

        if (params.n % npes != 0)
        {
            nColumnsPerProcess = ((int)(params.n / npes) + 1) * npes;
        }
        else
        {
            nColumnsPerProcess = params.n;
        }

        nRowsPerProcess = nColumnsPerProcess / npes;

        dataLength = nRowsPerProcess * nColumnsPerProcess;

        // Allocate buffers
        a = createMatrix(nRowsPerProcess, nColumnsPerProcess);
        s = createMatrix(nRowsPerProcess, nColumnsPerProcess);

        /*
        2- O processo P0 cria o vetor a com uma distribuição uniforme
        entre -1 e 1 com o auxílio da função rand() inicializada previamente
        com a semente.
        */
        if (myrank == 0)
        {
            // allocate memory
            aToSend = (double *)malloc(sizeof(double) * nColumnsPerProcess * nColumnsPerProcess);

            fillArrayWithRandom(&aToSend, nColumnsPerProcess * nColumnsPerProcess);

            for (long i = 0; i < nColumnsPerProcess; i++)
            {
                for (long j = 0; j < nColumnsPerProcess; j++)
                {
                    if ((i > params.n - 1) || (j > params.n - 1))
                    {
                        aToSend[i * nColumnsPerProcess + j] = 0.0;
                    }
                }
            }

            // Allocate sendcounts array
            sendcounts = malloc(npes * sizeof(int));
            // Allocate displacements array
            displs = malloc(npes * sizeof(int));
            for (int i = 0; i < npes; i++)
            {
                sendcounts[i] = dataLength;
                displs[i] = i * dataLength;
            }
        }

        // Scatter the a array by the processes using the distribution set
        // by the sendcounts and displs arrays
        MPI_Scatterv(aToSend,
                     sendcounts,
                     displs,
                     MPI_DOUBLE,
                     a->data,
                     dataLength,
                     MPI_DOUBLE,
                     0,
                     MPI_COMM_WORLD);

        if (myrank == 0)
        {
            free(aToSend);
            free(sendcounts);
            free(displs);
        }

        recvBuffer = (double *)malloc(sizeof(double) * dataLength);

        /**
         * M_k submatrix
         */
        Matrix *m;

        /**
         * Matrix to hold multiplied values and avoid having to allocate
         * and free memory every time we multiply the matrices
         */
        Matrix *multiplied = createMatrix(nRowsPerProcess, nColumnsPerProcess);

        /**
         * Matrix used to multiply A
         */
        Matrix *subMatrixA = createMatrix(nRowsPerProcess, nRowsPerProcess);

        // M1 = A
        m = duplicateMatrix(a);

        // S1 = I + M1
        setIdentitySubMatrix(&s, myrank * nRowsPerProcess, 0);
        sumMatrix(m, &s);

        long k = 2;
        int gonogo = PROCESS_CONTINUE;

        do
        {
            fillMatrixWithZeros(&multiplied);

            for (int p = 0; p < npes; p++)
            {
                if (p < npes - 1)
                {
                    // Send / retrieve the next m
                    MPI_Irecv(recvBuffer,
                              dataLength,
                              MPI_DOUBLE,
                              (myrank + 1) % npes,
                              MESSAGE_TAG_M_LINE,
                              MPI_COMM_WORLD,
                              &mRecvRequest);

                    MPI_Isend(m->data,
                              dataLength,
                              MPI_DOUBLE,
                              (npes + myrank - 1) % npes,
                              MESSAGE_TAG_M_LINE,
                              MPI_COMM_WORLD,
                              &mSendRequest);
                }

                copySubMatrix(&subMatrixA, a, 0, nRowsPerProcess, ((myrank + p) % npes) * nRowsPerProcess, nRowsPerProcess);
                multiplyMatrixAndSum(subMatrixA, m, &multiplied);

                if (p < npes - 1)
                {
                    MPI_Wait(&mRecvRequest, MPI_STATUS_IGNORE);
                    MPI_Wait(&mSendRequest, MPI_STATUS_IGNORE);

                    tmp = m->data;
                    m->data = recvBuffer;
                    recvBuffer = tmp;
                }
            }

            // M_k = A * M_k-1 / k
            tmp = m->data;
            m->data = multiplied->data;
            multiplied->data = tmp;

            divideMatrixByLong(&m, k);

            // S_k = S_k-1 + M_k
            sumMatrix(m, &s);

            double max = maxMij(m);

            // Stop or continue?
            if (myrank == 0)
            {
                MPI_Reduce(MPI_IN_PLACE,
                           &max,
                           1,
                           MPI_DOUBLE,
                           MPI_MAX,
                           0,
                           MPI_COMM_WORLD);

                if (max < params.tolerance)
                {
                    // Stop
                    gonogo = PROCESS_STOP;
                }
            }
            else
            {
                MPI_Reduce(&max,
                           &max,
                           1,
                           MPI_DOUBLE,
                           MPI_MAX,
                           0,
                           MPI_COMM_WORLD);
            }

            MPI_Bcast(
                &gonogo,
                1,
                MPI_INT,
                0,
                MPI_COMM_WORLD);

            k++;
        } while (gonogo == PROCESS_CONTINUE);

        res = OK;

        destroyMatrix(&subMatrixA);
        destroyMatrix(&m);
        destroyMatrix(&multiplied);
        free(recvBuffer);
    }

    // Sync everyone
    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0)
    {
        tf = MPI_Wtime();
        /* Elapsed time */
        printf("Elapsed time: %fs\n", tf - ti);
    }

    if (res == OK)
    {
        if (myrank == 0)
        {
            printMatrix("A", a, USE_SHORT_FORMAT);
            printMatrixToFile(params.outputfile, "A", a, USE_SHORT_FORMAT, OVERWRITE_FILE);

            for (int p = 1; p < npes; p++)
            {
                MPI_Recv(a->data,
                         dataLength,
                         MPI_DOUBLE,
                         p,
                         MESSAGE_TAG_A_LINE,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);

                printMatrix(NULL, a, USE_SHORT_FORMAT);
                printMatrixToFile(params.outputfile, NULL, a, USE_SHORT_FORMAT, APPEND_FILE);
            }

            printMatrix("S", s, USE_LONG_FORMAT);
            printMatrixToFile(params.outputfile, "S", s, USE_LONG_FORMAT, APPEND_FILE);

            for (int p = 1; p < npes; p++)
            {
                MPI_Recv(s->data,
                         dataLength,
                         MPI_DOUBLE,
                         p,
                         MESSAGE_TAG_S_FINAL_LINE,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);

                printMatrix(NULL, s, USE_LONG_FORMAT);
                printMatrixToFile(params.outputfile, NULL, s, USE_LONG_FORMAT, APPEND_FILE);
            }
        }
        else
        {
            MPI_Send(a->data,
                     dataLength,
                     MPI_DOUBLE,
                     0,
                     MESSAGE_TAG_A_LINE,
                     MPI_COMM_WORLD);

            MPI_Send(s->data,
                     dataLength,
                     MPI_DOUBLE,
                     0,
                     MESSAGE_TAG_S_FINAL_LINE,
                     MPI_COMM_WORLD);
        }
    }
    else
    {
        //TODO: something went wrong!
        printf("TODO: something went wrong!");
    }

    destroyMatrix(&a);
    destroyMatrix(&s);

    MPI_Finalize();
    return 0;
}
