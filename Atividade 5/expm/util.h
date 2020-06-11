#ifndef __UTIL_H__
#define __UTIL_H__

// default tolerance
#define DEFAULT_TOLERANCE 1e-5

// Cache threshold
//#define CACHE_THRESHOLD 49152 // 48KB L1 cache per processor
//#define CACHE_THRESHOLD 6142 // 48KB / 8 = 6KB
//#define CACHE_THRESHOLD 262144 // 256KB L2 cache per processor   
#define CACHE_THRESHOLD 32768 // 256KB / 8 = 32KB
//#define CACHE_THRESHOLD 1048576 // 1MB L3 cache per processor   
//#define CACHE_THRESHOLD 131072 // 1MB / 8 = 128KB

// Max for rand()
#define MAX_RAND_VALUE 1

// Min for rand()
#define MIN_RAND_VALUE -1

/**
 * Calculate random workload based on the #define's above
 */
#define RANDOM_VALUE ((double)rand() / RAND_MAX * (MAX_RAND_VALUE - MIN_RAND_VALUE) + MIN_RAND_VALUE)

// Tag to use on our messages
#define MESSAGE_TAG_M_LINE 1
#define MESSAGE_TAG_A_LINE 2
#define MESSAGE_TAG_S_FINAL_LINE 3

#define PROCESS_STOP 0
#define PROCESS_CONTINUE 1

#define USE_SHORT_FORMAT 0
#define SHORT_FORMAT " %7.4f"

#define USE_LONG_FORMAT 1
#define LONG_FORMAT " %11.4e"

#define OVERWRITE_FILE 0
#define APPEND_FILE 1

#define NOK 0
#define OK 1

#define MAX_ROWS_TO_OUTPUT 20
#define MAX_COLUMNS_TO_OUTPUT 20

#endif
