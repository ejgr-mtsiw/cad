#ifndef __PARSE_PARAM_H__
#define __PARSE_PARAM_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include "util.h"

typedef struct parse_param
{
    int seed;
    long n;
    char *outputfile;
    double tolerance;
} ParsedParams;

void printUsageMessage(const char *programName);

void printErrorAndExit(int rank, const char *programName, const char *message);

ParsedParams getParams(int rank, int argc, char *argv[]);

#endif
