#include "parse_param.h"

void printUsageMessage(const char *programName)
{
    printf("USAGE: %s -s seed -n dimension -o output-filename [-t tolerance]\n",
           programName);
}

void printErrorAndExit(int rank, const char *programName, const char *message)
{

    // Only process #0 reports the error to avoid spamming the command line
    if (rank == 0)
    {
        printf("%s\n", message);
        printUsageMessage(programName);
    }

    MPI_Finalize();
    exit(EXIT_SUCCESS);
}

ParsedParams getParams(int rank, int argc, char *argv[])
{
    int opt = 0, seed = 0;
    long n = 0;
    char *outputfilename = NULL;
    double tolerance = 0;

    ParsedParams params;
    params.tolerance = DEFAULT_TOLERANCE;

    // Check input arguments
    if (argc < 4)
    {
        printErrorAndExit(rank, argv[0], "Required arguments missing.");
    }

    while ((opt = getopt(argc, argv, "s:n:o:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            seed = atoi(optarg);

            if (seed <= 0)
            {
                printErrorAndExit(rank,
                                  argv[0],
                                  "Invalid seed value. Seed must be greater than 0");
            }

            params.seed = seed;
            break;
        case 'n':
            n = atol(optarg);
            if (n <= 0)
            {
                printErrorAndExit(rank,
                                  argv[0],
                                  "Invalid n value. N is the dimension of the matrix. Must be > 0.");
            }

            params.n = n;
            break;
        case 'o':
            outputfilename = optarg;
            if (strcmp(outputfilename, "") == 0)
            {
                printErrorAndExit(rank, argv[0], "Invalid output filename!");
            }

            params.outputfile = (char *)malloc(sizeof(char) * strlen(outputfilename));
            strcpy(params.outputfile, outputfilename);
            break;
        case 't':
            tolerance = atof(optarg);
            if (tolerance <= 0)
            {
                tolerance = DEFAULT_TOLERANCE;
            }

            params.tolerance = tolerance;
        }
    }

    return params;
}