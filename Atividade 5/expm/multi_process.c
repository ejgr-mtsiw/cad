#include "multi_process.h"

int multiProcess(
    ParsedParams *params,
    const Matrix *globalA,
    Matrix **globalS,
    int myrank,
    int npes)
{
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

    int res = OK;

    /**
     * MPI_Requests to control delivery
     */
    MPI_Request mSendRequest, mRecvRequest;

    // Data distribution
    nColumnsPerProcess = calculateColumnsPerProcess(params->n, npes);

    nRowsPerProcess = nColumnsPerProcess / npes;

    dataLength = nRowsPerProcess * nColumnsPerProcess;

    // Allocate buffers
    Matrix *a = createMatrixFilledWithZeros(nRowsPerProcess, nColumnsPerProcess);
    Matrix *s = createMatrixFilledWithZeros(nRowsPerProcess, nColumnsPerProcess);

    shareA(globalA, &a, myrank, npes);

    // Buffer for receving
    recvBuffer = (double *)malloc(sizeof(double) * dataLength);

    /**
     * Matrix to hold multiplied values and avoid having to allocate
     * and free memory every time we multiply the matrices
     */
    Matrix *multiplied = createMatrix(nRowsPerProcess, nColumnsPerProcess);

    long d = multiplied->nRows * multiplied->nColumns;
    double *zeroes = (double *)malloc(sizeof(double) * d);

    fillArrayWithZeros(&zeroes, d);

    /**
     * M_k submatrix
     * M1 = A
     */
    Matrix *m = duplicateMatrix(a);

    // S1 = I + M1
    setIdentitySubMatrix(&s, myrank * nRowsPerProcess, 0);
    sumMatrix(m, &s);

    long k = 2;
    int gonogo = PROCESS_CONTINUE;

    do
    {
        // Reset multiplication matrix
        memcpy(multiplied->data, zeroes, sizeof(double) * d);

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

            multiplyMatrixAndSumBlock(a,
                                      m,
                                      &multiplied,
                                      0,
                                      ((myrank + p) % npes) * nRowsPerProcess,
                                      0,
                                      0,
                                      0,
                                      0,
                                      a->nRows,
                                      m->nRows,
                                      m->nColumns);

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

            if (max <= params->tolerance)
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

    // Build final S matrix
    res = buildFinalSMatrix(globalS, s, myrank, npes);

    destroyMatrix(&a);
    destroyMatrix(&s);
    destroyMatrix(&m);
    destroyMatrix(&multiplied);
    free(recvBuffer);

    return res;
}

long calculateColumnsPerProcess(long n, int npes)
{

    if (n % npes == 0)
    {
        return n;
    }

    return ((n / npes) + 1) * npes;
}

int shareA(const Matrix *globalA, Matrix **a, int myrank, int npes)
{

    int *sendcounts, *displs;

    Matrix *aToSend = NULL;
    double *data = globalA->data;

    long dataLength = (*a)->nRows * (*a)->nColumns;

    if (myrank == 0)
    {
        // Allocate sendcounts array
        sendcounts = (int *)malloc(npes * sizeof(int));
        // Allocate displacements array
        displs = (int *)malloc(npes * sizeof(int));
        for (int i = 0; i < npes; i++)
        {
            sendcounts[i] = dataLength;
            displs[i] = i * dataLength;
        }

        // We need to adjust input data to our new internal size?
        if ((*a)->nColumns != globalA->nColumns)
        {
            aToSend = createMatrixFilledWithZeros(
                (*a)->nColumns,
                (*a)->nColumns);

            copySubMatrix(&aToSend,
                          globalA,
                          0,
                          0,
                          0,
                          0,
                          globalA->nRows,
                          globalA->nColumns);

            data = aToSend->data;
        }
    }

    // Scatter the a array by the processes using the distribution set
    // by the sendcounts and displs arrays
    MPI_Scatterv(data,
                 sendcounts,
                 displs,
                 MPI_DOUBLE,
                 (*a)->data,
                 dataLength,
                 MPI_DOUBLE,
                 0,
                 MPI_COMM_WORLD);

    if (myrank == 0)
    {
        destroyMatrix(&aToSend);
        free(sendcounts);
        free(displs);
    }

    return OK;
}

int buildFinalSMatrix(Matrix **globalS, Matrix *s, int myrank, int npes)
{

    long dataLength = s->nRows * s->nColumns;

    if (myrank == 0)
    {
        copySubMatrix(globalS,
                      s,
                      0,
                      0,
                      0,
                      0,
                      (*globalS)->nRows,
                      (*globalS)->nColumns);

        for (int p = 1; p < npes; p++)
        {
            if (s->nColumns == (*globalS)->nColumns)
            {
                MPI_Recv((*globalS)->data + p * dataLength,
                         dataLength,
                         MPI_DOUBLE,
                         p,
                         MESSAGE_TAG_S_FINAL_LINE,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
            }
            else
            {
                MPI_Recv(s->data,
                         dataLength,
                         MPI_DOUBLE,
                         p,
                         MESSAGE_TAG_S_FINAL_LINE,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);

                // We can't swap: the matrices don't have the same dimensions
                copySubMatrix(globalS,
                              s,
                              s->nRows * p,
                              0,
                              0,
                              0,
                              (*globalS)->nRows,
                              (*globalS)->nColumns);
            }
        }
    }
    else
    {
        MPI_Send(s->data,
                 dataLength,
                 MPI_DOUBLE,
                 0,
                 MESSAGE_TAG_S_FINAL_LINE,
                 MPI_COMM_WORLD);
    }

    return OK;
}