//
// Created by Vito Wu on 4/11/2021.
//

#ifndef MIMO_TOOLS_COMMON_HH
#define MIMO_TOOLS_COMMON_HH

#include <memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>  /* getpid() */
#include <unistd.h>
#include <assert.h>

/* for printf and sscanf specifiers for types in stdint.h */
#define __STDC_FORMAT_MACROS

#include <inttypes.h>

typedef uint64_t MTAPI;
#define MT_RET_SUCCESS  (0x1)
#define MT_RET_FAIL     (0x0)
#define MBFLOAT         (1024.0 * 1024.0)
#define MIN(a, b)       (((a) > (b)) ? (b) : (a))

#ifdef __cplusplus
extern "C" {
#endif

/* get timestamp to the precision of miliseconds since the program starts */
double get_timestamp();

/* print msg with timestamp */
#define PRINTF_STAMP(format, ...) \
        do { \
            flockfile(stdout); \
            printf("%12.2f - ", get_timestamp()); \
            printf(format, ##__VA_ARGS__); \
            fflush(stdout); \
            funlockfile(stdout); \
        } while(0)

/* print error msg to stderr */
#define PRINTF_ERR(format, ...) \
        do { \
            flockfile(stderr); \
            fprintf(stderr, format, ##__VA_ARGS__); \
            fflush(stderr); \
            funlockfile(stderr); \
        } while(0)

/* print error msg with timestamp to stderr */
#define PRINTF_ERR_STAMP(format, ...) \
        do { \
            flockfile(stderr); \
            fprintf(stderr, "%12.2f - ", get_timestamp()); \
            PRINTF_ERR(format, ##__VA_ARGS__); \
            funlockfile(stderr); \
        } while(0)

/* print error msg with timestamp to stderr then exit */
#define EXIT_WITH_MSG(format, ...) \
        do { \
            PRINTF_ERR_STAMP(format, ##__VA_ARGS__); \
            exit(-1); \
        } while (0)

/* print msg with timestamp to stderr if in debug mode */
#ifndef NDEBUG
#define PRINTF_DEBUG(format, ...) \
            PRINTF_ERR_STAMP(format, ##__VA_ARGS__)
#else
#define PRINTF_DEBUG(...)
#endif

/* print array to stderr if in debug mode */
#ifndef NDEBUG
#define PRINT_ARRAY_DEBUG(ele_format, array, size) \
            do { \
                unsigned int i; \
                fprintf(stderr, "%12.2f - array " #array ": ", get_timestamp()); \
                for(i = 0; i < (size); i++) { \
                    fprintf(stderr, ele_format, (array)[i]); \
                } \
                fprintf(stderr, "\n"); \
                fflush(stderr); \
            } while (0)
#else
#define PRINT_ARRAY_DEBUG(...)
#endif

/* function: safe_malloc
 * usage: abort if malloc failed
 * arguments: size, number of bytes to allocate
 * return: a void* pointer
 */
void *safe_malloc(size_t size);

/* macro: SMALLOC
 * usage: smart malloc, abort if malloc failed
 * arguments:
 *      1) type: type of the elements
 *      2) num: number of the elements
 * return: a type* pointer
 */
#define SMALLOC(type, num) \
        ((type*) safe_malloc((num) * sizeof(type)))

/* macro: SFREE
 * usage: smart free, release the address then set the pointer to NULL
 * arguments: a pointer whose memory chunk is to be released
 * return void
 */
#define SFREE(ptr) \
            do { \
                if(NULL != ptr) { \
                    free(ptr); \
                    ptr = NULL; \
                } \
            } while (0)

/* function: get_cpu_freq()
 * usage: read CPU frequency of the platform. Only Linux is supported.
 * arguments: void
 * return: CPU frequency and -1 on error
 */
double get_cpu_freq();

/* function: get_proc_status
 * usage: read the following info from /proc/$PID/status
 *      VmPeak          peak virtual memory size
 *      VmSize          total program size
 *      VmLck           locked memory size
 *      VmHWM           peak resident set size ("high water mark")
 *      VmRSS           size of memory portions
 *      VmData          size of data, stack, and text segments
 *      VmStk           size of data, stack, and text segments
 *      VmExe           size of text segment
 *      VmLib           size of shared library code
 *      VmPTE           size of page table entries
 *      VmSwap          size of swap usage (the number of referred swapents)
 * arguments:
 *      1) proc_info: a buffer to hold the info.
 *      2) size: size of the buffer
 * return: normally 0, -1 on error
 */
int get_proc_status(char *proc_info, size_t size);

#ifdef __cplusplus
}
#endif

#endif //MIMO_TOOLS_COMMON_HH
