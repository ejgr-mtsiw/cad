#ifndef __SINGLE_PROCESS_H__
#define __SINGLE_PROCESS_H__

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"
#include "parse_param.h"

int singleProcess(const ParsedParams *params, const Matrix *a, Matrix **s);

#endif
