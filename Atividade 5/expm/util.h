#ifndef __UTIL_H__
#define __UTIL_H__

// default tolerance
#define DEFAULT_TOLERANCE 0.00001f

// Max for rand()
#define MAX_RAND_VALUE 1

// Min for rand()
#define MIN_RAND_VALUE -1

/**
 * Calculate random workload based on the #define's above
 */
#define RANDOM_VALUE ((double)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE)

#define USE_SHORT_FORMAT 0
#define USE_LONG_FORMAT 1

#define SHORT_FORMAT " %7.4f"
#define LONG_FORMAT " %11.4e"

#define NOK 0
#define OK 1

#endif
