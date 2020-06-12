#ifndef __UTIL_H__
#define __UTIL_H__

// default tolerance
#define DEFAULT_TOLERANCE 1e-5

// Cache threshold
// Global:
// #define CACHE_THRESHOLD 786432   // 768KB L1 cache
// #define CACHE_THRESHOLD 4194304  // 4MB L2 cache
// #define CACHE_THRESHOLD 8388608  // 8MB L3 cache per CCX (Core Complex)
// #define CACHE_THRESHOLD 16777216 // 16MB L3 cache total

// SMT enabled: 16 processes
// #define CACHE_THRESHOLD 49152    // 48KB L1 cache per processor
// #define CACHE_THRESHOLD 6142     // 48KB / 8 = 6K doubles
// #define CACHE_THRESHOLD 262144   // 256KB L2 cache per processor
// #define CACHE_THRESHOLD 32768    // 256KB / 8 = 32K doubles
// #define CACHE_THRESHOLD 1048576  // 1MB L3 cache per processor
// #define CACHE_THRESHOLD 131072   // 1MB / 8 = 128K doubles

// SMT disabled: 8 processes
// #define CACHE_THRESHOLD 98304   // 96KB L1 cache per processor
// #define CACHE_THRESHOLD 12288   // 96KB / 8 = 12K doubles
// #define CACHE_THRESHOLD 524288  // 512KB L2 cache per processor
// #define CACHE_THRESHOLD 65536   // 512KB / 8 = 64K doubles
#define CACHE_THRESHOLD 2097152 // 2MB L3 cache per processor
// #define CACHE_THRESHOLD 262144  // 2MB / 8 = 256K doubles

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

//Formats for output matrix
#define USE_SHORT_FORMAT 0
#define SHORT_FORMAT " %7.4f"

#define USE_LONG_FORMAT 1
#define LONG_FORMAT " %11.4e"

#define MAX_ROWS_TO_OUTPUT 20
#define MAX_COLUMNS_TO_OUTPUT 20

#define OVERWRITE_FILE 0
#define APPEND_FILE 1

#define NOK 0
#define OK 1

#endif
